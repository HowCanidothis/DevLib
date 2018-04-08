#ifndef COMPUTENODEDELAY_H
#define COMPUTENODEDELAY_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeDelay : public GtComputeNodeBase
{
public:
    ComputeNodeDelay(const QString& name="ComputeNodeDelay");

    void setMotionMask(const cv::Mat* motion_mask);

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;
    size_t getMemoryUsage() const Q_DECL_OVERRIDE;
private:
    ScopedPointer<cv::Mat> back_buffer;
    const cv::Mat* motion_mask;
    FloatProperty intensity; // [0.f - 1.f]
};

#endif // COMPUTENODEDELAY_H
