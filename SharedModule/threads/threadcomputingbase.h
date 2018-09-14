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

    bool isStoped() const { return _stoped; }

protected:
    virtual void compute();  

private:
    std::atomic_bool _stoped;
    qint32 _idealFrameTime;

    QMutex _fpsLocker;
    ScopedPointer<TimerClocks> _fpsCounter;
    double _computeTime;
};

#endif // THREADCOMPUTING_H
