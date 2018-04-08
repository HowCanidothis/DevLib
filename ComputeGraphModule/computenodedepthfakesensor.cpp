#include "computenodedepthfakesensor.h"
#include "inputframestream.h"

#include <opencv2/opencv.hpp>

ComputeNodeDepthFakeSensor::ComputeNodeDepthFakeSensor(const QString& name)
    : GtComputeNodeBase(name)
    , input(nullptr)
{
}

void ComputeNodeDepthFakeSensor::setInputStream(InputFrameStream* stream)
{
    input = stream;
    onInputChanged(0);
}

bool ComputeNodeDepthFakeSensor::onInputChanged(const cv::Mat*)
{
    return true;
}

void ComputeNodeDepthFakeSensor::update(const cv::Mat*)
{
    if(input->readFrame()) {
        cv::Mat& output = *this->output;
        qint32 old_type = output.type();
        cv::MatSize old_size = output.size;
        output = input->getOutput();
        if(input->isValid() && old_type != output.type() || old_size != output.size) {
            outputChanged();
        }
    }
}
