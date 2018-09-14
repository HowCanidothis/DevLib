#include "threadcomputingbase.h"

#include <QMutexLocker>

#include "SharedModule/internal.hpp"

ThreadComputingBase::ThreadComputingBase(qint32 idealFrameTimeMsecs, QObject* parent)
    : QThread(parent)
    , _idealFrameTime(idealFrameTimeMsecs)
    , _fpsCounter(new TimerClocks)
{

}

ThreadComputingBase::~ThreadComputingBase()
{
    Quit();
}

void ThreadComputingBase::Start()
{
    _stoped = false;
    start();
}

void ThreadComputingBase::Quit()
{
    _stoped = true;
    wait();
}

double ThreadComputingBase::GetComputeTime()
{
    QMutexLocker locker(&_fpsLocker);
    return _computeTime;
}

void ThreadComputingBase::run()
{
    while (!_stoped) {
        auto guard = guards::make(this, &ThreadComputingBase::fpsBind, &ThreadComputingBase::fpsRelease);

        callEvents();

        compute();
    }
}

void ThreadComputingBase::compute()
{
    // Do compute here
}

void ThreadComputingBase::fpsBind()
{
    _fpsCounter->Bind();
}

void ThreadComputingBase::fpsRelease()
{
    qint32 msecs = Timer::ToMsecs(_fpsCounter->Release());
    qint32 dif = _idealFrameTime - msecs;
    {
        QMutexLocker locker(&_fpsLocker);
        _computeTime = _fpsCounter->CalculateMeanValue();
    }

    if(dif > 0) {
        msleep(dif);
    }
}
