#ifndef COMPUTENODEBILATERALBLUR_H
#define COMPUTENODEBILATERALBLUR_H

#include "computenodebase.h"
//deprecated since 18.01.2018
class ComputeNodeBilateralBlur : public GtComputeNodeBase
{
public:
    ComputeNodeBilateralBlur();

    void setParams(qint32 d, double sigmas);

    qint32 getD() const { return d; }
    qint32 getSigmas() const { return sigmas; }
    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

    qint32 d;
    double sigmas;
};

#endif // COMPUTENODEBILITERALBLUR_H
