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
    GtComputeGraph(qint32 ideal_frame_time = 30); //msecs
    ~GtComputeGraph();

    void asynch(GtComputeGraphEvent::FEventHandler handler);
    void processEvents();

    void start();
    void quit();

    void addCalculationGraph(GtComputeNodeBase* calculation_graph);

    double getComputeTime();

    // QThread interface
protected:
    void run();

private:
    StackPointers<GtComputeGraphEvent> events;
    Array<GtComputeNodeBase*> calculation_graphs;
    QMutex mutex;
    std::atomic_bool stoped;
    qint32 ideal_frame_time;

    QWaitCondition events_processed;
    QMutex events_mutex;
    std::atomic_bool events_notified;

    QMutex fps_locker;
    ScopedPointer<class TimerClocks> fps_counter;
    double _computeTime;
};

#endif // COMPUTEGRAPH_H
