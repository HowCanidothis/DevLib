#include "computegraphbase.h"
#include "computenodebase.h"
#include <QTimer>
#include <QThread>

#include "SharedModule/profile_utils.h"
#include "SharedModule/timer.h"

GtComputeGraph::GtComputeGraph(qint32 idealFrameTime)
    : ThreadComputingBase(idealFrameTime)
{

}

GtComputeGraph::~GtComputeGraph()
{

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

void GtComputeGraph::compute()
{
    QMutexLocker locker(&_mutex);
    for(GtComputeNodeBase* node : _calculationGraphs) {
        node->Compute(0);
    }
}
