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

GtComputeGraph::GtComputeGraph(qint32 ideal_frame_time)
    : ideal_frame_time(ideal_frame_time)
    , fps_counter(new TimerClocks)
{

}

GtComputeGraph::~GtComputeGraph()
{
    events.Clear();
    quit();
}

void GtComputeGraph::asynch(GtComputeGraphEvent::FEventHandler handler)
{
    QMutexLocker locker(&events_mutex);
    events.Push(new GtComputeGraphEvent(handler));
}

void GtComputeGraph::processEvents()
{
    QMutexLocker locker(&events_mutex);
    events_notified = false;
    while(!events_notified) { // from spurious wakeups
        events_processed.wait(&events_mutex);
    }
}

void GtComputeGraph::start()
{
    stoped = false;
    QThread::start();
}

void GtComputeGraph::quit()
{
    stoped = true;
    wait();
}

void GtComputeGraph::addCalculationGraph(GtComputeNodeBase* calculation_graph)
{
    if(isRunning()) {
        QMutexLocker locker(&mutex);
        calculation_graphs.Append(calculation_graph);
    } else {
        calculation_graphs.Append(calculation_graph);
    }

}

double GtComputeGraph::getComputeTime()
{
    QMutexLocker locker(&fps_locker);
    return _computeTime;
}

void GtComputeGraph::run()
{
    while (!stoped) {

        fps_counter->Bind();

        {
            QMutexLocker locker(&events_mutex);
            for(GtComputeGraphEvent* event : events) {
                event->call();
            }
            events.Clear();
            events_notified = true;
            events_processed.wakeAll();
        }
        {
            QMutexLocker locker(&mutex);
            for(GtComputeNodeBase* node : calculation_graphs) {
                node->compute(0);
            }
        }

        ;

        qint32 msecs = Timer::ToMsecs(fps_counter->Release());
        qint32 dif = ideal_frame_time - msecs;
        {
            QMutexLocker locker(&fps_locker);
            _computeTime = fps_counter->CalculateMeanValue();
        }

        if(dif > 0) {
            msleep(dif);
        }
    }
}


