#ifndef GTCOMPUTENODETHREADSAFE_H
#define GTCOMPUTENODETHREADSAFE_H

#include "computenodebase.h"
#include <QMutex>

class MatGuard
{
public:
    MatGuard(const cv::Mat* target, QMutex& mutex);
    ~MatGuard();

    const cv::Mat* getOutput() const { return target; }

private:
    const cv::Mat* target;
    QMutex* mutex;
};

class GtComputeNodeThreadSafe : public GtComputeNodeBase
{
public:
    GtComputeNodeThreadSafe(const QString& name="GtComputeNodeThreadSafe");

    const cv::Mat* getOutput() const { return nullptr; }
    MatGuard getThreadOutput() { return MatGuard(output.data(), mutex); }

    // GtComputeNodeBase interface
protected:
    size_t getMemoryUsage() const Q_DECL_OVERRIDE;
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    ScopedPointer<cv::Mat> back_buffer;
    QMutex mutex;
};

#endif // GTCOMPUTENODETHREADSAFEBASE_H
