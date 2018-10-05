#ifndef COMPUTEGRAPHBASE_H
#define COMPUTEGRAPHBASE_H

#include "SharedModule/internal.hpp"

class GtComputeNodeBase;

class GtComputeGraph : public ThreadComputingBase
{
    Q_OBJECT
public:
    GtComputeGraph(qint32 idealFrameTime = 30); //msecs
    ~GtComputeGraph();

    void AddCalculationGraph(GtComputeNodeBase* calculationGraph);

private:
    StackPointers<GtComputeNodeBase> _calculationGraphs;
    QMutex _mutex;

    // ThreadComputingBase interface
protected:
    virtual void compute() Q_DECL_OVERRIDE;
};

#endif // COMPUTEGRAPH_H
