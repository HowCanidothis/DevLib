#include "computenodevolcanorecognition.h"


ComputeNodeVolcanoRecognition::ComputeNodeVolcanoRecognition(const QString& name)
    : GtComputeNodeBase(name)
    , ideal_frames_per_second(name+"/ideal frame time", 30, 0, 30)
    , start_recognize_height(name+"/start recognize height", 2050, 2000, 3000)
    , range_recognize(name+"/range recognize", 100, 10, 150)
    , epsilon_recognize(name+"/epsilon recognize", 0, -10, 10)
    , current_frame(0)
{
    ideal_frames_per_second.onChange() = [this]{ current_frame = 0; };
}

void ComputeNodeVolcanoRecognition::update(const cv::Mat* input)
{
    if(current_frame != 0) {
        current_frame--;
        return;
    }
    current_frame = ideal_frames_per_second;

    cv::Mat gray_scale_input(input->size(), CV_8U);
    auto it_gray = gray_scale_input.begin<quint8>();
    for(quint16 pix : adapters::range(input->begin<quint16>(), input->end<quint16>())) {
        quint8& v = *it_gray;
        quint16 dif = pix - start_recognize_height;
        v = dif > range_recognize ? range_recognize : dif;
        it_gray++;
    }

    std::vector<cv::Vec3f> all_circles;
    cv::HoughCircles(gray_scale_input, all_circles, CV_HOUGH_GRADIENT, 1, 40, 20, 20, 10, 20);
    {
        QMutexLocker locker(&Mutex);
        Circles.clear();

        for(const cv::Vec3f& circle: all_circles) {
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
            qint32 height_center = input->at<quint16>(x, y);

            qint32* max = std::max_element(sides, sides + 4);

            if(*max && (height_center - *max + epsilon_recognize) > 0) {
                Circles.push_back(circle);
            }
        }
    }

    *output = gray_scale_input;
}

bool ComputeNodeVolcanoRecognition::onInputChanged(const cv::Mat* )
{
    return true;
}
