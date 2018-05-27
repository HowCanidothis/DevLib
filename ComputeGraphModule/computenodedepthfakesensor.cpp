#include "computenodedepthfakesensor.h"
#include "inputframestream.h"

#include <opencv2/opencv.hpp>

ComputeNodeDepthFakeSensor::ComputeNodeDepthFakeSensor(const QString& name)
    : GtComputeNodeBase(name)
    , _input(nullptr)
{
}

void ComputeNodeDepthFakeSensor::SetInputStream(InputFrameStream* stream)
{
    _input = stream;
    onInputChanged(0);
}

bool ComputeNodeDepthFakeSensor::onInputChanged(const cv::Mat*)
{
    return true;
}

void ComputeNodeDepthFakeSensor::update(const cv::Mat*)
{
    if(_input->ReadFrame()) {
        cv::Mat& output = *this->_output;
        qint32 oldType = output.type();
        cv::MatSize oldSize = output.size;
        output = _input->GetOutput();
        if(_input->IsValid() && oldType != output.type() || oldSize != output.size) {
            outputChanged();
        }
    }
}
