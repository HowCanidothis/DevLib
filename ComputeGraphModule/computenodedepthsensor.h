#ifndef COMPUTENODEDEPTHSENSOR_H
#define COMPUTENODEDEPTHSENSOR_H

#include "computenodebase.h"


class ComputeNodeDepthSensor : public GtComputeNodeBase
{
public:
    ComputeNodeDepthSensor(const QString& name="ComputeNodeDepthSensor");

protected:
    void update(const cv::Mat* input);
    bool onInputChanged(const cv::Mat* input);

private:
    ScopedPointer<class OpenniSensor> _sensor;
    bool _initialized;
};


#endif // COMPUTENODESENSOR_H
