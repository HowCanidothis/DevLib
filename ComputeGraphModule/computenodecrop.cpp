#include "computenodecrop.h"

#include <opencv2/opencv.hpp>

ComputeNodeCrop::ComputeNodeCrop(const QString& name)
    : GtComputeNodeBase(name)
    , _top(Name(name+"/top"), 0, 0, 600)
    , _left(Name(name+"/left"), 0, 0, 600)
    , _width(Name(name+"/width"), 300, 0, 600)
    , _height(Name(name+"/height"), 300, 0, 600)
{
}

bool ComputeNodeCrop::onInputChanged(const cv::Mat* input)
{
    _output->create(_height, _width, input->type());

    return true;
}

void ComputeNodeCrop::update(const cv::Mat* input)
{
    validateRect(_top, _left, _width, _height, input);
    cv::Rect rect(_left, _top, _width, _height);
    cv::Mat roi = (*input)(rect);
    *_output = roi.clone();
}

void ComputeNodeCrop::validateRect(IntProperty& top, IntProperty& left, IntProperty& width, IntProperty& height, const cv::Mat* input)
{
    qint32 max_width = input->cols;
    qint32 max_height = input->rows;
    left = qBound(0, left.Native(), max_width - 1);
    top = qBound(0, top.Native(), max_height - 1);
    width = qBound(1, width.Native(), max_width - left);
    height = qBound(1, height.Native(), max_height - top);
    if(_output->rows != height || _output->cols != width) {
        _output->create(height, width, input->type());
        outputChanged();
    }
}
