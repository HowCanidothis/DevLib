#include "computenodebase.h"
#include <opencv2/opencv.hpp>

GtComputeNodeBase::GtComputeNodeBase(const QString& name, qint32 flags)
    : _inputNode(nullptr)
    , _output(new cv::Mat)
    , _name(name)
    , _enabled(name+"/enabled", true)
{
    this->_flags = flags;

    _enabled.OnChange() = [this]{ SetEnabled(_enabled); };
}

GtComputeNodeBase::~GtComputeNodeBase()
{

}

void GtComputeNodeBase::SetName(const QString& name)
{
    this->_name = name;
}

const QString&GtComputeNodeBase::GetName() const
{
    return _name;
}

void GtComputeNodeBase::SetEnabled(bool flag)
{
    if(IsSkippeable()) {
        LOGOUT;
        log.Warning() << _name << flag;
        _enabled = flag;
        updateLater();
    }
    else {
        setEnabledAllOutputs();
    }
}

void GtComputeNodeBase::Compute(const cv::Mat* input)
{
#ifndef QT_NO_DEBUG
    QString name = this->_name.section('(', 0, 0);
    name += QString("(%1 MB)").arg(QString::number(double(getMemoryUsage()) / 1000000));
    SetName(name);
#endif
    if(_flags.TestFlag(F_NeedUpdate)) {
        if(!onInputChanged(input) && !IsSkippeable()) {
            return;
        }
        outputChanged();
        _flags.UnsetFlag(F_NeedUpdate);
    }
    if(_enabled) {
        update(input);
    }
    else if(!IsSkippeable()) {
        return;
    }
    for(GtComputeNodeBase* node : _linkedOutputs)
        node->Compute(this->GetOutput());

}

void GtComputeNodeBase::SetInput(GtComputeNodeBase* node)
{
    if(_inputNode) {
        _inputNode->_linkedOutputs.RemoveByPredicate([this](const GtComputeNodeBase* cn) {

            return cn == this;
        });
    }
    _inputNode = node;
    node->_linkedOutputs.Append(this);
    updateLater();
}

const cv::Mat* GtComputeNodeBase::GetOutput() const
{
    return _enabled ? _output.data() : _inputNode ? _inputNode->GetOutput() : nullptr;
}

size_t GtComputeNodeBase::getMemoryUsage() const
{
    if(!IsSkippeable()) {
        size_t total = _output->total() * _output->elemSize();
        totalMemoryUsage(this, total);
        return total;
    }
    return _output->total() * _output->elemSize();
}

void GtComputeNodeBase::updateLater()
{
    _flags.SetFlag(F_NeedUpdate);
}

void GtComputeNodeBase::outputChanged()
{
    for(GtComputeNodeBase* node : _linkedOutputs) {
        if(!node->onInputChanged(this->GetOutput()))
            node->SetEnabled(false);
        else
            node->outputChanged();
    }
}

void GtComputeNodeBase::setEnabledAllOutputs()
{
    for(GtComputeNodeBase* node : _linkedOutputs) {
        node->SetEnabled(!node->IsEnabled());
        node->setEnabledAllOutputs();
    }
}

void GtComputeNodeBase::totalMemoryUsage(const GtComputeNodeBase* parent, size_t& result) const
{
    for(GtComputeNodeBase* node : parent->_linkedOutputs) {
        result += node->getMemoryUsage();
        totalMemoryUsage(node, result);
    }
}
