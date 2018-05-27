#ifndef COMPUTENODEHOLESFILTER_H
#define COMPUTENODEHOLESFILTER_H

#include "computenodebase.h"

class ComputeNodeHolesFilter : public GtComputeNodeBase
{
public:
    ComputeNodeHolesFilter(const QString& name="ComputeNodeHolesFilter");

protected:
    void update(const cv::Mat* input);
    bool onInputChanged(const cv::Mat* input);

private:
    void horizontalInterpolation(const cv::Mat& input, cv::Mat& out);
};

#endif // COMPUTENODEHOLESFILTER_H
