#ifndef COMPUTENODEVOLCANORECOGNITION_H
#define COMPUTENODEVOLCANORECOGNITION_H

#include "computenodebase.h"
#include <opencv2/opencv.hpp>

#include "PropertiesModule/property.h"

#include <QMutex>

class ComputeNodeVolcanoRecognition : public GtComputeNodeBase
{
public:
    ComputeNodeVolcanoRecognition(const QString& _name="ComputeNodeVolcanoRecognition");

    void SetParams(qint32 idealFramesPerSecond);

    qint32 getIdealFramesPerSecond() const { return _idealFramesPerSecond.Native(); }

    QMutex Mutex;
    std::vector<cv::Vec3f> Circles;

    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    IntProperty _idealFramesPerSecond;
    UIntProperty _startRecognizeHeight;
    UIntProperty _rangeRecognize;
    IntProperty _epsilonRecognize;
    quint32 _currentFrame;
};

#endif // COMPUTENODEVOLCANORECOGNITION_H
