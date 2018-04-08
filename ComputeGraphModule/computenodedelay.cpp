#include "computenodedelay.h"

#include <opencv2/opencv.hpp>

ComputeNodeDelay::ComputeNodeDelay(const QString& name)
    : GtComputeNodeBase(name)
    , back_buffer(new cv::Mat)
    , motion_mask(nullptr)
    , intensity(name+"/intensity", 0.1f, 0.f, 1.f)
{

}

void ComputeNodeDelay::setMotionMask(const cv::Mat* mask)
{
    this->motion_mask = mask;
}

bool ComputeNodeDelay::onInputChanged(const cv::Mat* input)
{
    *output = input->clone();
    back_buffer->create(input->size(), input->type());
    return true;
}

void ComputeNodeDelay::update(const cv::Mat* in)
{
    const cv::Mat& previous = *this->output;
    const cv::Mat& input = *in;
    cv::Mat& output = *back_buffer;

    auto it_in = input.begin<quint16>();
    auto it_in_e = input.end<quint16>();
    auto it_prev = previous.begin<quint16>();
    auto it_out = output.begin<quint16>();

    if(motion_mask == nullptr) {
        for(; it_in != it_in_e; it_in++, it_prev++, it_out++) {
            quint16& out_value = *it_out;
            quint16 in_value = *it_in;
            quint16 prev_value = *it_prev;

            quint16 offset = (intensity * (in_value - prev_value));
            out_value = prev_value + offset;
        }
    }
    else {
        const cv::Mat& mask = *this->motion_mask;
        float x_ratio = float(mask.rows) / input.rows;
        float y_ratio = float(mask.cols) / input.cols;
        qint32 r = 0;
        for(; it_in != it_in_e; it_in++, it_prev++, it_out++, r++) {
            qint32 row = r / input.cols;
            qint32 col = r % input.cols;
            quint16 prev_value = *it_prev;
            quint16& out_value = *it_out;

            quint8 mask_value = mask.at<quint8>(x_ratio * row, y_ratio * col);
            if(mask_value) {
                quint16 in_value = *it_in;

                quint16 offset = (intensity * (in_value - prev_value));
                out_value = prev_value + offset;
            }
            else {
                out_value = prev_value;
            }
        }
    }

    cv::swap(*this->output, output);
}

size_t ComputeNodeDelay::getMemoryUsage() const
{
    return GtComputeNodeBase::getMemoryUsage() + back_buffer->total() * back_buffer->elemSize();
}
