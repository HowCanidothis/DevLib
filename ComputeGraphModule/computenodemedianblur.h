#ifndef COMPUTENODEMEDIANBLUR_H
#define COMPUTENODEMEDIANBLUR_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeMedianBlur : public GtComputeNodeBase
{
public:
    ComputeNodeMedianBlur(const QString& name="ComputeNodeMedianBlur");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    IntProperty _kSize;
};

#endif // COMPUTENODEMEDIUMBLUR_H
