#ifndef GTCOMPUTENODETHREADSAFE_H
#define GTCOMPUTENODETHREADSAFE_H

#include "computenodebase.h"
#include <QMutex>

class MatGuard
{
public:
    MatGuard(const cv::Mat* target, QMutex& mutex);
    ~MatGuard();

    const cv::Mat* GetOutput() const { return _target; }

private:
    const cv::Mat* _target;
    QMutex* _mutex;
};

class GtComputeNodeThreadSafe : public GtComputeNodeBase
{
public:
    GtComputeNodeThreadSafe(const QString& _name="GtComputeNodeThreadSafe");

    const cv::Mat* GetOutput() const { return nullptr; }
    MatGuard GetThreadOutput() { return MatGuard(_output.data(), _mutex); }

    // GtComputeNodeBase interface
protected:
    size_t getMemoryUsage() const Q_DECL_OVERRIDE;
    bool onInputChanged(const cv::Mat* input) Q_DECL_OVERRIDE;
    void update(const cv::Mat* input) Q_DECL_OVERRIDE;

private:
    ScopedPointer<cv::Mat> _backBuffer;
    QMutex _mutex;
};

#endif // GTCOMPUTENODETHREADSAFEBASE_H
