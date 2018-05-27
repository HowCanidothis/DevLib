#include "computenodedelay.h"

#include <opencv2/opencv.hpp>

ComputeNodeDelay::ComputeNodeDelay(const QString& name)
    : GtComputeNodeBase(name)
    , _backBuffer(new cv::Mat)
    , _motionMask(nullptr)
    , _intensity(name+"/intensity", 0.1f, 0.f, 1.f)
{

}

void ComputeNodeDelay::SetMotionMask(const cv::Mat* mask)
{
    this->_motionMask = mask;
}

bool ComputeNodeDelay::onInputChanged(const cv::Mat* input)
{
    *_output = input->clone();
    _backBuffer->create(input->size(), input->type());
    return true;
}

void ComputeNodeDelay::update(const cv::Mat* in)
{
    const cv::Mat& previous = *this->_output;
    const cv::Mat& input = *in;
    cv::Mat& output = *_backBuffer;

    auto itIn = input.begin<quint16>();
    auto itInE = input.end<quint16>();
    auto itPrev = previous.begin<quint16>();
    auto itOut = output.begin<quint16>();

    if(_motionMask == nullptr) {
        for(; itIn != itInE; itIn++, itPrev++, itOut++) {
            quint16& outValue = *itOut;
            quint16 in_value = *itIn;
            quint16 prevValue = *itPrev;

            quint16 offset = (_intensity * (in_value - prevValue));
            outValue = prevValue + offset;
        }
    }
    else {
        const cv::Mat& mask = *this->_motionMask;
        float xRatio = float(mask.rows) / input.rows;
        float yRatio = float(mask.cols) / input.cols;
        qint32 r = 0;
        for(; itIn != itInE; itIn++, itPrev++, itOut++, r++) {
            qint32 row = r / input.cols;
            qint32 col = r % input.cols;
            quint16 prevValue = *itPrev;
            quint16& outValue = *itOut;

            quint8 maskValue = mask.at<quint8>(xRatio * row, yRatio * col);
            if(maskValue) {
                quint16 in_value = *itIn;

                quint16 offset = (_intensity * (in_value - prevValue));
                outValue = prevValue + offset;
            }
            else {
                outValue = prevValue;
            }
        }
    }

    cv::swap(*this->_output, output);
}

size_t ComputeNodeDelay::getMemoryUsage() const
{
    return GtComputeNodeBase::getMemoryUsage() + _backBuffer->total() * _backBuffer->elemSize();
}
