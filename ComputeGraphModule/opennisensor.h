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

    openni::Status Initialize();

    bool CreateOutput(openni::SensorType, qint32 video_mode_index=0);
    void Update();
    void Start();
    void Stop();

    const cv::Mat* GetOutput(openni::SensorType type) const { return &_outputs[type - 1]; }

private:
    openni::VideoStream*& input(openni::SensorType type) { return _inputs[type - 1]; }
    cv::Mat& output(openni::SensorType type) { return _outputs[type - 1]; }

private:
    enum { MaxSensors = openni::SENSOR_DEPTH };
    openni::Device* _device=nullptr;
    openni::VideoStream* _inputs[MaxSensors];
    cv::Mat _outputs[MaxSensors];
    openni::Status _rc;
};

#endif // OPENNITOIMAGE_H
