#include "computenoderesize.h"

#include <opencv2/opencv.hpp>

ComputeNodeResize::ComputeNodeResize(const QString& name)
    : GtComputeNodeBase(name)
    , width(name+"/width", 64, 1, 600)
    , height(name+"/height", 64, 1, 600)
{
    width.onChange() = [this]{ updateLater(); };
    height.onChange() = [this]{ updateLater(); };
}

bool ComputeNodeResize::onInputChanged(const cv::Mat* input)
{
    output->create(cv::Size(width, height), input->type());
    return true;
}

void ComputeNodeResize::update(const cv::Mat* input)
{
    cv::resize(*input, *output, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
}
