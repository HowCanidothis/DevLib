#include "computegraphbase.h"
#include "computenodebase.h"
#include <QTimer>
#include <QThread>

#include "Shared/profile_utils.h"
#include "Shared/timer.h"

void GtComputeGraphEvent::call()
{
    handler();
}

GtComputeGraph::GtComputeGraph(qint32 idealFrameTime)
    : _idealFrameTime(idealFrameTime)
    , _fpsCounter(new TimerClocks)
{

}

GtComputeGraph::~GtComputeGraph()
{
    _events.Clear();
    Quit();
}

void GtComputeGraph::Asynch(GtComputeGraphEvent::FEventHandler handler)
{
    QMutexLocker locker(&_eventsMutex);
    _events.Push(new GtComputeGraphEvent(handler));
}

void GtComputeGraph::ProcessEvents()
{
    QMutexLocker locker(&_eventsMutex);
    _eventsNotified = false;
    while(!_eventsNotified) { // from spurious wakeups
        _eventsProcessed.wait(&_eventsMutex);
    }
}

void GtComputeGraph::Start()
{
    _stoped = false;
    QThread::start();
}

void GtComputeGraph::Quit()
{
    _stoped = true;
    wait();
}

void GtComputeGraph::AddCalculationGraph(GtComputeNodeBase* calculationGraph)
{
    if(isRunning()) {
        QMutexLocker locker(&_mutex);
        _calculationGraphs.Append(calculationGraph);
    } else {
        _calculationGraphs.Append(calculationGraph);
    }

}

double GtComputeGraph::GetComputeTime()
{
    QMutexLocker locker(&_fpsLocker);
    return _computeTime;
}

void GtComputeGraph::run()
{
    while (!_stoped) {

        _fpsCounter->Bind();

        {
            QMutexLocker locker(&_eventsMutex);
            for(GtComputeGraphEvent* event : _events) {
                event->call();
            }
            _events.Clear();
            _eventsNotified = true;
            _eventsProcessed.wakeAll();
        }
        {
            QMutexLocker locker(&_mutex);
            for(GtComputeNodeBase* node : _calculationGraphs) {
                node->Compute(0);
            }
        }

        ;

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
}


