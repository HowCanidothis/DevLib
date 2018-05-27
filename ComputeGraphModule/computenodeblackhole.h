#ifndef COMPUTENODEBLACKHOLE_H
#define COMPUTENODEBLACKHOLE_H

#include "computenodebase.h"
#include "PropertiesModule/property.h"

class ComputeNodeBlackHole : public GtComputeNodeBase
{
public:
    ComputeNodeBlackHole(const QString& name = "ComputeNodeBlackHole");

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    IntProperty _minHeight;
};

#endif // COMPUTENODEBLACKHOLE_H
