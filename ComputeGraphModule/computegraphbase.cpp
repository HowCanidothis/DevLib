#include "computegraphbase.h"
#include "computenodebase.h"
#include <QTimer>
#include <QThread>

#include "SharedModule/profile_utils.h"
#include "SharedModule/timer.h"

GtComputeGraph::GtComputeGraph(qint32 idealFrameTime)
    : _idealFrameTime(idealFrameTime)
    , _fpsCounter(new TimerClocks)
{

}

GtComputeGraph::~GtComputeGraph()
{
    Quit();
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

        CallEvents();

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


