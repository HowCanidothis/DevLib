#include "computenodevolcanorecognition.h"


ComputeNodeVolcanoRecognition::ComputeNodeVolcanoRecognition(const QString& name)
    : GtComputeNodeBase(name)
    , _idealFramesPerSecond(name+"/ideal frame time", 30, 0, 30)
    , _startRecognizeHeight(name+"/start recognize height", 2050, 2000, 3000)
    , _rangeRecognize(name+"/range recognize", 100, 10, 150)
    , _epsilonRecognize(name+"/epsilon recognize", 0, -10, 10)
    , _currentFrame(0)
{
    _idealFramesPerSecond.OnChange() = [this]{ _currentFrame = 0; };
}

void ComputeNodeVolcanoRecognition::update(const cv::Mat* input)
{
    if(_currentFrame != 0) {
        _currentFrame--;
        return;
    }
    _currentFrame = _idealFramesPerSecond;

    cv::Mat grayScaleInput(input->size(), CV_8U);
    auto itGray = grayScaleInput.begin<quint8>();
    for(quint16 pix : adapters::range(input->begin<quint16>(), input->end<quint16>())) {
        quint8& v = *itGray;
        quint16 dif = pix - _startRecognizeHeight;
        v = dif > _rangeRecognize ? _rangeRecognize : dif;
        itGray++;
    }

    std::vector<cv::Vec3f> allCircles;
    cv::HoughCircles(grayScaleInput, allCircles, CV_HOUGH_GRADIENT, 1, 40, 20, 20, 10, 20);
    {
        QMutexLocker locker(&Mutex);
        Circles.clear();

        for(const cv::Vec3f& circle: allCircles) {
            float y = circle[0];
            float x = circle[1];
            float r = circle[2];

            Point2I right(x, std::min((qint32)(y + r), input->cols - 1));
            Point2I bottom(std::max((qint32)(x - r), 0), y);
            Point2I left(x, std::max((qint32)(y - r), 0));
            Point2I top(std::min((qint32)(x + r), input->rows - 1), y);

            static qint32 sides[4];

            sides[Right] = input->at<quint16>(right.x(), right.y());
            sides[Bottom] = input->at<quint16>(bottom.x(), bottom.y());
            sides[Left] = input->at<quint16>(left.x(), left.y());
            sides[Top] = input->at<quint16>(top.x(), top.y());
            qint32 heightCenter = input->at<quint16>(x, y);

            qint32* max = std::max_element(sides, sides + 4);

            if(*max && (heightCenter - *max + _epsilonRecognize) > 0) {
                Circles.push_back(circle);
            }
        }
    }

    *_output = grayScaleInput;
}

bool ComputeNodeVolcanoRecognition::onInputChanged(const cv::Mat* )
{
    return true;
}
