#ifndef COMPUTEGRAPHBASE_H
#define COMPUTEGRAPHBASE_H

#include <QMutex>
#include "SharedModule/internal.hpp"
#include <atomic>
#include <QThread>

#ifdef WORK
#include <functional>
#endif

class GtComputeNodeBase;
class QTimer;

class GtComputeGraph : public QThread, public ThreadEventsContainer
{
    Q_OBJECT
public:
    GtComputeGraph(qint32 idealFrameTime = 30); //msecs
    ~GtComputeGraph();

    void Start();
    void Quit();

    void AddCalculationGraph(GtComputeNodeBase* calculationGraph);

    double GetComputeTime();

    // QThread interface
protected:
    void run();

    void start() {}

private:
    Array<GtComputeNodeBase*> _calculationGraphs;
    QMutex _mutex;
    std::atomic_bool _stoped;
    qint32 _idealFrameTime;

    QMutex _fpsLocker;
    ScopedPointer<class TimerClocks> _fpsCounter;
    double _computeTime;
};

#endif // COMPUTEGRAPH_H
