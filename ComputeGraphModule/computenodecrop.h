#ifndef COMPUTENODECROP_H
#define COMPUTENODECROP_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeCrop : public GtComputeNodeBase
{
public:
    ComputeNodeCrop(const QString& _name="ComputeNodeCrop");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    IntProperty _top;
    IntProperty _left;
    IntProperty _width;
    IntProperty _height;

    void validateRect(IntProperty& top, IntProperty& left, IntProperty& width, IntProperty& height, const cv::Mat* input);
};

#endif // COMPUTENODECROP_H
