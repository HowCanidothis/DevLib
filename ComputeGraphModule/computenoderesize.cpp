#include "computenoderesize.h"

#include <opencv2/opencv.hpp>

ComputeNodeResize::ComputeNodeResize(const QString& name)
    : GtComputeNodeBase(name)
    , _width(name+"/width", 64, 1, 600)
    , _height(name+"/height", 64, 1, 600)
{
    _width.OnChange() = [this]{ updateLater(); };
    _height.OnChange() = [this]{ updateLater(); };
}

bool ComputeNodeResize::onInputChanged(const cv::Mat* input)
{
    _output->create(cv::Size(_width, _height), input->type());
    return true;
}

void ComputeNodeResize::update(const cv::Mat* input)
{
    cv::resize(*input, *_output, cv::Size(_width, _height), 0, 0, cv::INTER_LINEAR);
}
