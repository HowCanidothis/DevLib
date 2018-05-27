#include "computenodegaussenblur.h"
#include <opencv2/opencv.hpp>
#include "PropertiesModule/propertiesvalidators.h"

ComputeNodeGaussenBlur::ComputeNodeGaussenBlur(const QString& name)
    : GtComputeNodeBase(name)
    , x_kernels(name+"/x_kernels", 21, 1, 255)
    , y_kernels(name+"/y_kernels", 21, 1, 255)
{
    PropertiesValidators v;
    x_kernels.Validator() = v.OddValidator<quint32>();
    y_kernels.Validator() = v.OddValidator<quint32>();
}

bool ComputeNodeGaussenBlur::onInputChanged(const cv::Mat* input)
{
    *output = input->clone();
    return true;
}

void ComputeNodeGaussenBlur::update(const cv::Mat* input)
{
    cv::Mat& mat = *output;
    cv::GaussianBlur(*input, mat, {x_kernels,y_kernels}, 0,0);
}
