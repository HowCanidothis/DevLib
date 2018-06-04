#include "opennisensor.h"
#include <OpenNI.h>
#include <QDebug>
#include <QTimer>
#include "SharedModule/decl.h"

OpenniSensor::OpenniSensor()
    : _device(new openni::Device)
{
    for(openni::VideoStream*& input : _inputs) {
        input = nullptr;
    }
}

OpenniSensor::~OpenniSensor()
{
    for(openni::VideoStream* input : _inputs) {
        if(input) {
            input->stop();
            input->destroy();
            delete input;
        }
    }

    _device->close();

    openni::OpenNI::shutdown();
}

openni::Status OpenniSensor::Initialize()
{
    LOGOUT;
    _rc = openni::OpenNI::initialize();
    if (_rc != openni::STATUS_OK)
    {
        log.Error("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
        openni::OpenNI::shutdown();
        return _rc;
    }

    _rc = _device->open(openni::ANY_DEVICE);
    if (_rc != openni::STATUS_OK)
    {
        log.Error("Couldn't open device\n%s\n", openni::OpenNI::getExtendedError());
        return _rc;
    }

    log.Info("Openni initialized");
    return _rc;
}

bool OpenniSensor::CreateOutput(openni::SensorType type, qint32 videoModeIndex)
{
    LOGOUT;
    if (_device->getSensorInfo(type) != NULL)
    {
        openni::VideoStream*& stream = input(type);
        if(stream != nullptr)
            return false;
        stream = new openni::VideoStream;
        _rc = stream->create(*_device, type);

        if (_rc != openni::STATUS_OK)
        {
            log.Error("Couldn't create depth stream\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }

        _rc = stream->start();
        if (_rc != openni::STATUS_OK)
        {
            log.Error("Couldn't start the depth stream\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }

        const openni::SensorInfo& sensorInfo = stream->getSensorInfo();
        log.Info("Supported video modes:\n");
        const openni::Array<openni::VideoMode>& supportedModes = sensorInfo.getSupportedVideoModes();
        for(qint32 i(0); i < supportedModes.getSize(); i++) {
            const openni::VideoMode& vm = supportedModes[i];
            log.Info("fps:%d pf:%d x:%d y:%d\n", vm.getFps(), vm.getPixelFormat(), vm.getResolutionX(), vm.getResolutionY());
        }
        log.Info("chossed %d", videoModeIndex);

        stream->setVideoMode(supportedModes[videoModeIndex]);

        const openni::VideoMode& currentVm = supportedModes[videoModeIndex];
        cv::Mat& img = output(type);
        qint32 imgFormat;
        switch(currentVm.getPixelFormat()) {
            case openni::PIXEL_FORMAT_DEPTH_1_MM:
            case openni::PIXEL_FORMAT_DEPTH_100_UM:
            case openni::PIXEL_FORMAT_SHIFT_9_2:
            case openni::PIXEL_FORMAT_SHIFT_9_3: imgFormat = CV_16UC1; break;

            // Color
            case openni::PIXEL_FORMAT_RGB888: imgFormat = CV_8UC3; break;
            case openni::PIXEL_FORMAT_YUV422:
            case openni::PIXEL_FORMAT_GRAY8:
            case openni::PIXEL_FORMAT_GRAY16:
            case openni::PIXEL_FORMAT_JPEG:
            case openni::PIXEL_FORMAT_YUYV:

            default: log.Error("unnacpected pixel format"); return false;
        };
        img.create(cv::Size(currentVm.getResolutionX(), currentVm.getResolutionY()), imgFormat);

        return true;
    }
    return false;
}

void OpenniSensor::Start()
{
    for(openni::VideoStream* stream : _inputs)
        if(stream != nullptr) {
            stream->start();
        }
}

void OpenniSensor::Stop()
{
    for(openni::VideoStream* stream : _inputs)
        if(stream != nullptr) {
            stream->stop();
        }
}

void OpenniSensor::Update()
{
    int changedStreamDummy;
    for(qint32 i(0); i < MaxSensors; i++) {

        openni::Status rc = openni::OpenNI::waitForAnyStream(_inputs, MaxSensors, &changedStreamDummy, 0);
        if (rc != openni::STATUS_OK)
        {
            return;
        }

        openni::VideoStream* stream = _inputs[changedStreamDummy];
        openni::VideoFrameRef frameRef;
        stream->readFrame(&frameRef);
        cv::Mat& img = output((openni::SensorType)(changedStreamDummy + 1));
        uchar* dst = img.data;
        const openni::DepthPixel* src = (const openni::DepthPixel*)frameRef.getData();

        // Size
        qint32 stride = frameRef.getStrideInBytes();

        // Foreach lines.
        for (int y = 0; y < frameRef.getHeight(); ++y) {
            const openni::DepthPixel* p = src + frameRef.getWidth() - 1;
            const openni::DepthPixel* l = src - 1;
            openni::DepthPixel* linear = (openni::DepthPixel*)dst;
            while(p != l) {
                *linear = *p;
                linear++;
                p--;
            }

            // Offset.
            src += frameRef.getWidth();
            dst += stride;
        }
    }
}
