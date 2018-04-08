#include "computenodebilateralblur.h"
#include <opencv2/opencv.hpp>

ComputeNodeBilateralBlur::ComputeNodeBilateralBlur()
    : GtComputeNodeBase("ComputeNodeBiliteralBlur")
    , d(5)
    , sigmas(150)
{
}

void ComputeNodeBilateralBlur::setParams(qint32 d, double sigmas)
{
    this->d = d;
    this->sigmas = sigmas;
}

bool ComputeNodeBilateralBlur::onInputChanged(const cv::Mat* input)
{
    *output = input->clone();
    return true;
}

void ComputeNodeBilateralBlur::update(const cv::Mat* input)
{
    cv::Mat& mat = *output;
    cv::bilateralFilter(*input, mat, d, sigmas, sigmas);
}
