#include "computenodedepthsensor.h"
#include "opennisensor.h"

#include <QThread>

ComputeNodeDepthSensor::ComputeNodeDepthSensor(const QString& name)
    : GtComputeNodeBase(name, F_Default | F_NeedUpdate)
    , sensor(new OpenniSensor())
    , initialized(false)
{
    qint32 tryings = 30;
    while(tryings-- && !initialized) {
        initialized = sensor->initialize() == openni::STATUS_OK;
        QThread::msleep(1);
    }

    if(!initialized) {
        LOGOUT;
        log.warning("Available sensors not detected");
    }
}

bool ComputeNodeDepthSensor::onInputChanged(const cv::Mat*)
{
    if(initialized) {
        sensor->createOutput(openni::SENSOR_DEPTH, 0);
        *output = sensor->getOutput(openni::SENSOR_DEPTH)->clone();
        return true;
    }
    return false;
}

void ComputeNodeDepthSensor::update(const cv::Mat*)
{
    sensor->update();
    sensor->getOutput(openni::SENSOR_DEPTH)->copyTo(*output);
}


