#include "computenodethreadsafe.h"
#include <opencv2/opencv.hpp>

MatGuard::MatGuard(const cv::Mat* target, QMutex& mutex)
    : target(target)
    , mutex(&mutex)
{
    mutex.lock();
}

MatGuard::~MatGuard()
{
    mutex->unlock();
}

GtComputeNodeThreadSafe::GtComputeNodeThreadSafe(const QString& name)
    : GtComputeNodeBase(name)
    , back_buffer(new cv::Mat)
{

}

size_t GtComputeNodeThreadSafe::getMemoryUsage() const
{
    return GtComputeNodeBase::getMemoryUsage() + back_buffer->total() * back_buffer->elemSize();
}

bool GtComputeNodeThreadSafe::onInputChanged(const cv::Mat* input)
{
    if(input->size() != output->size()) {
        QMutexLocker locker(&mutex);
        *output = input->clone();
        *back_buffer = input->clone();
    }
    return true;
}

void GtComputeNodeThreadSafe::update(const cv::Mat* input)
{
    *back_buffer = input->clone();

    {
        QMutexLocker locker(&mutex);
        cv::swap(*back_buffer, *output);
    }
}
