#include "computenodeblur.h"
#include <opencv2/opencv.hpp>

ComputeNodeBlur::ComputeNodeBlur(const QString& name)
    : GtComputeNodeBase(name)
    , _xKernels(name+"/x_kernels", 6, 1, 250)
    , _yKernels(name+"/y_kernels", 6, 1, 250)
{
}

bool ComputeNodeBlur::onInputChanged(const cv::Mat* input)
{
    *_output = input->clone();
    return true;
}

void ComputeNodeBlur::update(const cv::Mat* input)
{
    cv::Mat& mat = *_output;
    cv::blur(*input, mat, {_xKernels,_yKernels}, {-1, -1});
}
