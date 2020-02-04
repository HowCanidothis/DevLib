#ifndef THREADCOMPUTINGBASE_H
#define THREADCOMPUTINGBASE_H

#include <QThread>

#include "threadeventshelper.h"
#include "SharedModule/smartpointersadapters.h"

class TimerClocks;

class _Export ThreadComputingBase : public QThread, public ThreadEventsContainer
{
    typedef QThread Super;
public:
    ThreadComputingBase(qint32 idealFrameTimeMsecs = 30, QObject* parent = nullptr);
    virtual ~ThreadComputingBase() Q_DECL_OVERRIDE;

    void Start();
    void Quit();

    double GetComputeTime();

    // QThread interface
protected:
    virtual void run() Q_DECL_OVERRIDE;

    void start() { Super::start(); }
    void quit() { Super::quit(); }

    void fpsBind();
    void fpsRelease();

    bool isStoped() const { return m_stoped; }

protected:
    virtual void compute();  

private:
    std::atomic_bool m_stoped;
    qint32 m_idealFrameTime;

    QMutex m_fpsLocker;
    ScopedPointer<TimerClocks> m_fpsCounter;
    double m_computeTime;
};

#endif // THREADCOMPUTING_H
