#ifndef COMPUTENODEMINRESIZE_H
#define COMPUTENODEMINRESIZE_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"
//deprecated since 18.01.2018
class ComputeNodeMinResize : public GtComputeNodeBase
{
public:
    ComputeNodeMinResize(const QString& _name="ComputeNodeMinResize");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    UIntProperty _width;
    UIntProperty _height;

    float _xRatio;
    float _yRatio;
};


#endif // COMPUTENODEMINRESIZE_H
