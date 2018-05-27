#ifndef COMPUTENODEBLUR_H
#define COMPUTENODEBLUR_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeBlur : public GtComputeNodeBase
{
public:
    ComputeNodeBlur(const QString& name="ComputeNodeBlur");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    UIntProperty x_kernels;
    UIntProperty y_kernels;
};

#endif // COMPUTENODEBLUR_H
