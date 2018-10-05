#include "computenodemedianblur.h"
#include <opencv2/opencv.hpp>
#include "PropertiesModule/propertiesvalidators.h"

ComputeNodeMedianBlur::ComputeNodeMedianBlur(const QString& name)
    : GtComputeNodeBase(name)
    , _kSize(Name(name+"/ksize"), 5, 1, 7)
{
    _kSize.Validator() = PropertiesValidators::OddValidator<qint32>();
}

bool ComputeNodeMedianBlur::onInputChanged(const cv::Mat* input)
{
    *_output = input->clone();
    return true;
}

void ComputeNodeMedianBlur::update(const cv::Mat* input)
{
    cv::Mat& mat = *_output;
    cv::medianBlur(*input, mat, _kSize);
}
