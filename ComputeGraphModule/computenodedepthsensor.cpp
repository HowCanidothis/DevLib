#include "computenodedepthsensor.h"
#include "opennisensor.h"

#include <QThread>

ComputeNodeDepthSensor::ComputeNodeDepthSensor(const QString& name)
    : GtComputeNodeBase(name, F_Default | F_NeedUpdate)
    , _sensor(new OpenniSensor())
    , _initialized(false)
{
    qint32 tryings = 30;
    while(tryings-- && !_initialized) {
        _initialized = _sensor->Initialize() == openni::STATUS_OK;
        QThread::msleep(1);
    }

    if(!_initialized) {
        qCWarning(LC_SYSTEM) << "Available sensors not detected";
    }
}

bool ComputeNodeDepthSensor::onInputChanged(const cv::Mat*)
{
    if(_initialized) {
        _sensor->CreateOutput(openni::SENSOR_DEPTH, 0);
        *_output = _sensor->GetOutput(openni::SENSOR_DEPTH)->clone();
        return true;
    }
    return false;
}

void ComputeNodeDepthSensor::update(const cv::Mat*)
{
    _sensor->Update();
    _sensor->GetOutput(openni::SENSOR_DEPTH)->copyTo(*_output);
}


