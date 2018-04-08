#ifndef COMPUTENODEDEPTHSENSOR_H
#define COMPUTENODEDEPTHSENSOR_H

#include "computenodebase.h"


class ComputeNodeDepthSensor : public GtComputeNodeBase
{
    ScopedPointer<class OpenniSensor> sensor;
    bool initialized;
public:
    ComputeNodeDepthSensor(const QString& name="ComputeNodeDepthSensor");

protected:
    void update(const cv::Mat* input);
    bool onInputChanged(const cv::Mat* input);
};


#endif // COMPUTENODESENSOR_H
