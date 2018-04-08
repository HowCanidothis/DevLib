#include "opennisensor.h"
#include <OpenNI.h>
#include <QDebug>
#include <QTimer>
#include "Shared/shared_decl.h"

OpenniSensor::OpenniSensor() : device(new openni::Device)
{
    for(openni::VideoStream*& input : inputs)
        input = nullptr;
}

OpenniSensor::~OpenniSensor()
{
    for(openni::VideoStream* input : inputs) {
        if(input) {
            input->stop();
            input->destroy();
            delete input;
        }
    }

    device->close();

    openni::OpenNI::shutdown();
}

openni::Status OpenniSensor::initialize()
{
    LOGOUT;
    rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK)
    {
        log.error("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
        openni::OpenNI::shutdown();
        return rc;
    }

    rc = device->open(openni::ANY_DEVICE);
    if (rc != openni::STATUS_OK)
    {
        log.error("Couldn't open device\n%s\n", openni::OpenNI::getExtendedError());
        return rc;
    }

    log.info("Openni initialized");
    return rc;
}

bool OpenniSensor::createOutput(openni::SensorType type, qint32 video_mode_index)
{
    LOGOUT;
    if (device->getSensorInfo(type) != NULL)
    {
        openni::VideoStream*& stream = input(type);
        if(stream != nullptr)
            return false;
        stream = new openni::VideoStream;
        rc = stream->create(*device, type);

        if (rc != openni::STATUS_OK)
        {
            log.error("Couldn't create depth stream\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }

        rc = stream->start();
        if (rc != openni::STATUS_OK)
        {
            log.error("Couldn't start the depth stream\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }

        const openni::SensorInfo& sensor_info = stream->getSensorInfo();
        log.info("Supported video modes:\n");
        const openni::Array<openni::VideoMode>& supported_modes = sensor_info.getSupportedVideoModes();
        for(qint32 i(0); i < supported_modes.getSize(); i++) {
            const openni::VideoMode& vm = supported_modes[i];
            log.info("fps:%d pf:%d x:%d y:%d\n", vm.getFps(), vm.getPixelFormat(), vm.getResolutionX(), vm.getResolutionY());
        }
        log.info("chossed %d", video_mode_index);

        stream->setVideoMode(supported_modes[video_mode_index]);

        const openni::VideoMode& current_vm = supported_modes[video_mode_index];
        cv::Mat& img = output(type);
        qint32 img_format;
        switch(current_vm.getPixelFormat()) {
            case openni::PIXEL_FORMAT_DEPTH_1_MM:
            case openni::PIXEL_FORMAT_DEPTH_100_UM:
            case openni::PIXEL_FORMAT_SHIFT_9_2:
            case openni::PIXEL_FORMAT_SHIFT_9_3: img_format = CV_16UC1; break;

            // Color
            case openni::PIXEL_FORMAT_RGB888: img_format = CV_8UC3; break;
            case openni::PIXEL_FORMAT_YUV422:
            case openni::PIXEL_FORMAT_GRAY8:
            case openni::PIXEL_FORMAT_GRAY16:
            case openni::PIXEL_FORMAT_JPEG:
            case openni::PIXEL_FORMAT_YUYV:

            default: log.error("unnacpected pixel format"); return false;
        };
        img.create(cv::Size(current_vm.getResolutionX(), current_vm.getResolutionY()), img_format);

        return true;
    }
    return false;
}

void OpenniSensor::start()
{
    for(openni::VideoStream* stream : inputs)
        if(stream != nullptr) {
            stream->start();
        }
}

void OpenniSensor::stop()
{
    for(openni::VideoStream* stream : inputs)
        if(stream != nullptr) {
            stream->stop();
        }
}

void OpenniSensor::update()
{
    int changedStreamDummy;
    for(qint32 i(0); i < MaxSensors; i++) {

        openni::Status rc = openni::OpenNI::waitForAnyStream(inputs, MaxSensors, &changedStreamDummy, 0);
        if (rc != openni::STATUS_OK)
        {
            return;
        }

        openni::VideoStream* stream = inputs[changedStreamDummy];
        openni::VideoFrameRef frame_ref;
        stream->readFrame(&frame_ref);
        cv::Mat& img = output((openni::SensorType)(changedStreamDummy + 1));
        uchar* dst = img.data;
        const openni::DepthPixel* src = (const openni::DepthPixel*)frame_ref.getData();

        // Size
        qint32 stride = frame_ref.getStrideInBytes();

        // Foreach lines.
        for (int y = 0; y < frame_ref.getHeight(); ++y)
        {
            const openni::DepthPixel* p = src + frame_ref.getWidth() - 1;
            const openni::DepthPixel* l = src - 1;
            openni::DepthPixel* linear = (openni::DepthPixel*)dst;
            while(p != l) {
                *linear = *p;
                linear++;
                p--;
            }

            // Offset.
            src += frame_ref.getWidth();
            dst += stride;
        }
    }
}
