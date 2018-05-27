#ifndef COMPUTENODERESIZE_H
#define COMPUTENODERESIZE_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeResize : public GtComputeNodeBase
{
public:
    ComputeNodeResize(const QString& name="ComputeNodeResize");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    UIntProperty _width;
    UIntProperty _height;
};

#endif // COMPUTENODERESIZE_H
