#ifndef COMPUTEGRAPHBASE_H
#define COMPUTEGRAPHBASE_H

#include <QMutex>
#include "Shared/array.h"
#include "Shared/stack.h"
#include <atomic>
#include <QThread>
#include <QWaitCondition>

#ifdef WORK
#include <functional>
#endif

class GtComputeNodeBase;
class QTimer;

class GtComputeGraphEvent
{
public:
    typedef std::function<void()> FEventHandler;

    GtComputeGraphEvent(FEventHandler  handler)
        : handler(handler)
    {}
private:
    friend class GtComputeGraph;
    FEventHandler handler;

    void call();
};

class GtComputeGraph : public QThread
{
    Q_OBJECT
public:
    GtComputeGraph(qint32 idealFrameTime = 30); //msecs
    ~GtComputeGraph();

    void Asynch(GtComputeGraphEvent::FEventHandler handler);
    void ProcessEvents();

    void Start();
    void Quit();

    void AddCalculationGraph(GtComputeNodeBase* calculationGraph);

    double GetComputeTime();

    // QThread interface
protected:
    void run();

    void start() {}

private:
    StackPointers<GtComputeGraphEvent> _events;
    Array<GtComputeNodeBase*> _calculationGraphs;
    QMutex _mutex;
    std::atomic_bool _stoped;
    qint32 _idealFrameTime;

    QWaitCondition _eventsProcessed;
    QMutex _eventsMutex;
    std::atomic_bool _eventsNotified;

    QMutex _fpsLocker;
    ScopedPointer<class TimerClocks> _fpsCounter;
    double _computeTime;
};

#endif // COMPUTEGRAPH_H
