#include "computenodemedianblur.h"
#include <opencv2/opencv.hpp>
#include "PropertiesModule/propertiesvalidators.h"

ComputeNodeMedianBlur::ComputeNodeMedianBlur(const QString& name)
    : GtComputeNodeBase(name)
    , ksize(name+"/ksize", 5, 1, 7)
{
    ksize.validator() = PropertiesValidators::oddValidator<qint32>();
}

bool ComputeNodeMedianBlur::onInputChanged(const cv::Mat* input)
{
    *output = input->clone();
    return true;
}

void ComputeNodeMedianBlur::update(const cv::Mat* input)
{
    cv::Mat& mat = *output;
    cv::medianBlur(*input, mat, ksize);
}
