#include "computenodebase.h"
#include <opencv2/opencv.hpp>

GtComputeNodeBase::GtComputeNodeBase(const QString& name, qint32 flags)
    : input_node(nullptr)
    , output(new cv::Mat)
    , name(name)
    , enabled(name+"/enabled", true)
{
    this->flags = flags;

    enabled.onChange() = [this]{ setEnabled(enabled); };
}

GtComputeNodeBase::~GtComputeNodeBase()
{

}

void GtComputeNodeBase::setName(const QString& name)
{
    this->name = name;
}

const QString&GtComputeNodeBase::getName() const
{
    return name;
}

void GtComputeNodeBase::setEnabled(bool flag)
{
    if(isSkippeable()) {
        LOGOUT;
        log.warning() << name << flag;
        enabled = flag;
        updateLater();
    }
    else {
        setEnabledAllOutputs();
    }
}

void GtComputeNodeBase::compute(const cv::Mat* input)
{
#ifndef QT_NO_DEBUG
    QString name = this->name.section('(', 0, 0);
    name += QString("(%1 MB)").arg(QString::number(double(getMemoryUsage()) / 1000000));
    setName(name);
#endif
    if(flags.testFlag(F_NeedUpdate)) {
        if(!onInputChanged(input) && !isSkippeable()) {
            return;
        }
        outputChanged();
        flags.unsetFlag(F_NeedUpdate);
    }
    if(enabled) {
        update(input);
    }
    else if(!isSkippeable()) {
        return;
    }
    for(GtComputeNodeBase* node : linked_outputs)
        node->compute(this->getOutput());

}

void GtComputeNodeBase::setInput(GtComputeNodeBase* node)
{
    if(input_node) {
        input_node->linked_outputs.removeByPredicate([this](const GtComputeNodeBase* cn) {

            return cn == this;
        });
    }
    input_node = node;
    node->linked_outputs.append(this);
    updateLater();
}

const cv::Mat* GtComputeNodeBase::getOutput() const
{
    return enabled ? output.data() : input_node ? input_node->getOutput() : nullptr;
}

size_t GtComputeNodeBase::getMemoryUsage() const
{
    if(!isSkippeable()) {
        size_t total = output->total() * output->elemSize();
        totalMemoryUsage(this, total);
        return total;
    }
    return output->total() * output->elemSize();
}

void GtComputeNodeBase::updateLater()
{
    flags.setFlag(F_NeedUpdate);
}

void GtComputeNodeBase::outputChanged()
{
    for(GtComputeNodeBase* node : linked_outputs) {
        if(!node->onInputChanged(this->getOutput()))
            node->setEnabled(false);
        else
            node->outputChanged();
    }
}

void GtComputeNodeBase::setEnabledAllOutputs()
{
    for(GtComputeNodeBase* node : linked_outputs) {
        node->setEnabled(!node->isEnabled());
        node->setEnabledAllOutputs();
    }
}

void GtComputeNodeBase::totalMemoryUsage(const GtComputeNodeBase* parent, size_t& result) const
{
    for(GtComputeNodeBase* node : parent->linked_outputs) {
        result += node->getMemoryUsage();
        totalMemoryUsage(node, result);
    }
}
