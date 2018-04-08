#include "computenodeblur.h"
#include <opencv2/opencv.hpp>

ComputeNodeBlur::ComputeNodeBlur(const QString& name)
    : GtComputeNodeBase(name)
    , x_kernels(name+"/x_kernels", 6, 1, 250)
    , y_kernels(name+"/y_kernels", 6, 1, 250)
{
}

bool ComputeNodeBlur::onInputChanged(const cv::Mat* input)
{
    *output = input->clone();
    return true;
}

void ComputeNodeBlur::update(const cv::Mat* input)
{
    cv::Mat& mat = *output;
    cv::blur(*input, mat, {x_kernels,y_kernels}, {-1, -1});
}
