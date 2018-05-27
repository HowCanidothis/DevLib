#ifndef COMPUTENODEDELAY_H
#define COMPUTENODEDELAY_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeDelay : public GtComputeNodeBase
{
public:
    ComputeNodeDelay(const QString& name="ComputeNodeDelay");

    void SetMotionMask(const cv::Mat* motionMask);

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;
    size_t getMemoryUsage() const Q_DECL_OVERRIDE;

private:
    ScopedPointer<cv::Mat> _backBuffer;
    const cv::Mat* _motionMask;
    FloatProperty _intensity; // [0.f - 1.f]
};

#endif // COMPUTENODEDELAY_H
