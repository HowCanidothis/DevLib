#include "computenodeminresize.h"
#include <opencv2/opencv.hpp>

ComputeNodeMinResize::ComputeNodeMinResize(const QString& name)
    : GtComputeNodeBase(name)
    , _width(Name(name+"/width"), 64, 1, 600)
    , _height(Name(name+"/height"), 64, 1, 600)
{
    _width.OnChange() = [this]{ updateLater(); };
    _height.OnChange() = [this]{ updateLater(); };
}

bool ComputeNodeMinResize::onInputChanged(const cv::Mat* input)
{
    _output->create(cv::Size(_width, _height), input->type());
    _xRatio = float(_output->rows) / input->rows;
    _yRatio = float(_output->cols) / input->cols;
    *_output = UINT16_MAX;
    return true;
}

void ComputeNodeMinResize::update(const cv::Mat* in)
{
    const cv::Mat& input = *in;
    cv::Mat& output = *this->_output;
    output = UINT16_MAX;

    qint32 r = 0;
    auto itIn = input.begin<quint16>();
    auto itInE = input.end<quint16>();

    for(; itIn != itInE; itIn++, r++) {
        qint32 row = r / input.cols;
        qint32 col = r % input.cols;
        const quint16& inValue = *itIn;
        quint16& outValue = output.at<quint16>(_xRatio * row, _yRatio * col);
        outValue = std::min(inValue, outValue);
    }
}
