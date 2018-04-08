#include "computenodeminresize.h"
#include <opencv2/opencv.hpp>

ComputeNodeMinResize::ComputeNodeMinResize(const QString& name)
    : GtComputeNodeBase(name)
    , width(name+"/width", 64, 1, 600)
    , height(name+"/height", 64, 1, 600)
{
    width.onChange() = [this]{ updateLater(); };
    height.onChange() = [this]{ updateLater(); };
}

bool ComputeNodeMinResize::onInputChanged(const cv::Mat* input)
{
    output->create(cv::Size(width, height), input->type());
    x_ratio = float(output->rows) / input->rows;
    y_ratio = float(output->cols) / input->cols;
    *output = UINT16_MAX;
    return true;
}

void ComputeNodeMinResize::update(const cv::Mat* in)
{
    const cv::Mat& input = *in;
    cv::Mat& output = *this->output;
    output = UINT16_MAX;

    qint32 r = 0;
    auto it_in = input.begin<quint16>();
    auto it_in_e = input.end<quint16>();

    for(; it_in != it_in_e; it_in++, r++) {
        qint32 row = r / input.cols;
        qint32 col = r % input.cols;
        const quint16& in_value = *it_in;
        quint16& out_value = output.at<quint16>(x_ratio * row, y_ratio * col);
        out_value = std::min(in_value, out_value);
    }
}
