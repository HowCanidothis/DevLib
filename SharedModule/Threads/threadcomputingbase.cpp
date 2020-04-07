#include "threadcomputingbase.h"

#include <QMutexLocker>

#include "SharedModule/internal.hpp"

ThreadComputingBase::ThreadComputingBase(qint32 idealFrameTimeMsecs, QObject* parent)
    : QThread(parent)
    , m_idealFrameTime(idealFrameTimeMsecs)
    , m_fpsCounter(new TimerClocks)
{

}

ThreadComputingBase::~ThreadComputingBase()
{
    Quit();
}

void ThreadComputingBase::Start()
{
    m_stoped = false;
    start();
}

void ThreadComputingBase::Quit()
{
    clearEvents();
    m_stoped = true;
    wait();
}

double ThreadComputingBase::GetComputeTime()
{
    QMutexLocker locker(&m_fpsLocker);
    return m_computeTime;
}

void ThreadComputingBase::run()
{
    while (!m_stoped) {
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
    m_fpsCounter->Bind();
}

void ThreadComputingBase::fpsRelease()
{
    qint32 msecs = Timer::ToMsecs(m_fpsCounter->Release());
    qint32 dif = m_idealFrameTime - msecs;
    {
        QMutexLocker locker(&m_fpsLocker);
        m_computeTime = m_fpsCounter->CalculateMeanValue();
    }

    if(dif > 0) {
        msleep(dif);
    }
}
