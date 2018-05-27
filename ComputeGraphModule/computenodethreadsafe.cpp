#include "computenodethreadsafe.h"
#include <opencv2/opencv.hpp>

MatGuard::MatGuard(const cv::Mat* target, QMutex& mutex)
    : _target(target)
    , _mutex(&mutex)
{
    mutex.lock();
}

MatGuard::~MatGuard()
{
    _mutex->unlock();
}

GtComputeNodeThreadSafe::GtComputeNodeThreadSafe(const QString& name)
    : GtComputeNodeBase(name)
    , _backBuffer(new cv::Mat)
{

}

size_t GtComputeNodeThreadSafe::getMemoryUsage() const
{
    return GtComputeNodeBase::getMemoryUsage() + _backBuffer->total() * _backBuffer->elemSize();
}

bool GtComputeNodeThreadSafe::onInputChanged(const cv::Mat* input)
{
    if(input->size() != _output->size()) {
        QMutexLocker locker(&_mutex);
        *_output = input->clone();
        *_backBuffer = input->clone();
    }
    return true;
}

void GtComputeNodeThreadSafe::update(const cv::Mat* input)
{
    *_backBuffer = input->clone();

    {
        QMutexLocker locker(&_mutex);
        cv::swap(*_backBuffer, *_output);
    }
}
