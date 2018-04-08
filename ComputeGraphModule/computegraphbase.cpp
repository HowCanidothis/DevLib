#include "computegraphbase.h"
#include "computenodebase.h"
#include <QTimer>
#include <QThread>

#include "Shared/profile_utils.h"

void GtComputeGraphEvent::call()
{
    handler();
}

GtComputeGraph::GtComputeGraph(qint32 ideal_frame_time)
    : ideal_frame_time(ideal_frame_time)
    , fps_counter(new FPSCounter)
{

}

GtComputeGraph::~GtComputeGraph()
{
    events.clear();
    quit();
}

void GtComputeGraph::asynch(GtComputeGraphEvent::FEventHandler handler)
{
    QMutexLocker locker(&events_mutex);
    events.push(new GtComputeGraphEvent(handler));
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
    QMutexLocker locker(&mutex);
    calculation_graphs.append(calculation_graph);
}

float GtComputeGraph::getComputeTime()
{
    QMutexLocker locker(&fps_locker);
    return fps_counter->findMeanFPS();
}

void GtComputeGraph::run()
{
    while (!stoped) {
        Timer local_timer;
//        Timers::print();

        Timers::bind(TimerEnum::Apply);
        {
            QMutexLocker locker(&events_mutex);
            for(GtComputeGraphEvent* event : events) {
                event->call();
            }
            events.clear();
            events_notified = true;
            events_processed.wakeAll();
        }
        {
            QMutexLocker locker(&mutex);
            for(GtComputeNodeBase* node : calculation_graphs) {
                node->compute(0);
            }
        }
        qint32 msecs = Timer::toMsecs(local_timer.release());
        qint32 dif = ideal_frame_time - msecs;
        Timers::set(TimerEnum::Apply);
        {
            QMutexLocker locker(&fps_locker);
            fps_counter->add(Timers::get(TimerEnum::Apply));
        }
        if(dif > 0) {
            msleep(dif);
        }
    }
}


