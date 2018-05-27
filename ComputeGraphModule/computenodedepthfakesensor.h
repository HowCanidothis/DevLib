#ifndef COMPUTENODEDEPTHFAKESENSOR_H
#define COMPUTENODEDEPTHFAKESENSOR_H

#include "computenodebase.h"

class InputFrameStream;

class ComputeNodeDepthFakeSensor : public GtComputeNodeBase
{
public:
    ComputeNodeDepthFakeSensor(const QString& name="ComputeNodeDepthFakeSensor");

    void SetInputStream(InputFrameStream* stream);

    // GtComputeNodeBase interface
protected:
    virtual bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    virtual void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    InputFrameStream* _input;
};

#endif // COMPUTENODEDEPTHFAKESENSOR_H
