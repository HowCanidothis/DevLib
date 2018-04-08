#ifndef COMPUTENODEMINRESIZE_H
#define COMPUTENODEMINRESIZE_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"
//deprecated since 18.01.2018
class ComputeNodeMinResize : public GtComputeNodeBase
{
public:
    ComputeNodeMinResize(const QString& name="ComputeNodeMinResize");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

    UIntProperty width;
    UIntProperty height;

    float x_ratio;
    float y_ratio;
};


#endif // COMPUTENODEMINRESIZE_H
