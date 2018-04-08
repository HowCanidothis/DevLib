#ifndef COMPUTENODEDEPTHFAKESENSOR_H
#define COMPUTENODEDEPTHFAKESENSOR_H

#include "computenodebase.h"

class InputFrameStream;

class ComputeNodeDepthFakeSensor : public GtComputeNodeBase
{
    InputFrameStream* input;
public:
    ComputeNodeDepthFakeSensor(const QString& name="ComputeNodeDepthFakeSensor");

    void setInputStream(InputFrameStream* stream);

    // GtComputeNodeBase interface
protected:
    virtual bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    virtual void update(const cv::Mat* input) Q_DECL_OVERRIDE;
};

#endif // COMPUTENODEDEPTHFAKESENSOR_H
