#include "computenodegaussenblur.h"
#include <opencv2/opencv.hpp>
#include "PropertiesModule/propertiesvalidators.h"

ComputeNodeGaussenBlur::ComputeNodeGaussenBlur(const QString& name)
    : GtComputeNodeBase(name)
    , _xKernels(name+"/x_kernels", 21, 1, 255)
    , _yKernels(name+"/y_kernels", 21, 1, 255)
{
    PropertiesValidators v;
    _xKernels.Validator() = v.OddValidator<quint32>();
    _yKernels.Validator() = v.OddValidator<quint32>();
}

bool ComputeNodeGaussenBlur::onInputChanged(const cv::Mat* input)
{
    *_output = input->clone();
    return true;
}

void ComputeNodeGaussenBlur::update(const cv::Mat* input)
{
    cv::Mat& mat = *_output;
    cv::GaussianBlur(*input, mat, {_xKernels,_yKernels}, 0,0);
}
