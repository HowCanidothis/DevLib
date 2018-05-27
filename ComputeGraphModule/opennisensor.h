#ifndef OPENNISENSOR_H
#define OPENNISENSOR_H

#include <Qt>
#include <OniEnums.h>
#include <opencv2/opencv.hpp>

namespace openni {
    class VideoStream;
    class Device;
}

class OpenniSensor
{
public:
    OpenniSensor();
    ~OpenniSensor();

    openni::Status initialize();

    bool createOutput(openni::SensorType, qint32 video_mode_index=0);
    void update();
    void start();
    void stop();

    const cv::Mat* getOutput(openni::SensorType type) const { return &outputs[type - 1]; }

private:
    openni::VideoStream*& input(openni::SensorType type) { return inputs[type - 1]; }
    cv::Mat& output(openni::SensorType type) { return outputs[type - 1]; }

private:
    enum { MaxSensors = openni::SENSOR_DEPTH };
    openni::Device* device=nullptr;
    openni::VideoStream* inputs[MaxSensors];
    cv::Mat outputs[MaxSensors];
    openni::Status rc;
};

#endif // OPENNITOIMAGE_H
