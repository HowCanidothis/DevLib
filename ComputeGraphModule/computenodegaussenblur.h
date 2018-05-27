#ifndef COMPUTENODEGAUSSENBLUR_H
#define COMPUTENODEGAUSSENBLUR_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeGaussenBlur : public GtComputeNodeBase
{
public:
    ComputeNodeGaussenBlur(const QString& name="ComputeNodeGaussenBlur");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    UIntProperty _xKernels;
    UIntProperty _yKernels;
};

#endif // COMPUTENODEGAUSSENBLUR_H
