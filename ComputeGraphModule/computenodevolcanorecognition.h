#ifndef COMPUTENODEVOLCANORECOGNITION_H
#define COMPUTENODEVOLCANORECOGNITION_H

#include "computenodebase.h"
#include <opencv2/opencv.hpp>

#include "PropertiesModule/property.h"

#include <QMutex>

class ComputeNodeVolcanoRecognition : public GtComputeNodeBase
{
public:
    ComputeNodeVolcanoRecognition(const QString& name="ComputeNodeVolcanoRecognition");

    void setParams(qint32 ideal_frames_per_second);

    qint32 getIdealFramesPerSecond() const { return ideal_frames_per_second; }

    QMutex Mutex;
    std::vector<cv::Vec3f> Circles;

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    UIntProperty ideal_frames_per_second;
    UIntProperty start_recognize_height;
    UIntProperty range_recognize;
    IntProperty epsilon_recognize;
    quint32 current_frame;
};

#endif // COMPUTENODEVOLCANORECOGNITION_H
