#ifndef COMPUTENODEBASE_H
#define COMPUTENODEBASE_H

#include "Shared/array.h"
#include "Shared/flags.h"
#include "SharedGui/decl.h"

#include "PropertiesModule/property.h"

class GtComputeGraphEvent;

namespace cv {
    class Mat;
}

class GtComputeNodeBase
{
protected:
    enum Flag
    {
        F_Default = 0x0,
        F_NeedUpdate = 0x1,
    };
    DECL_FLAGS(Flags, Flag)

    GtComputeNodeBase* input_node;
    Array<GtComputeNodeBase*> linked_outputs;
    ScopedPointer<cv::Mat> output;
    Flags flags;
    QString name;
    BoolProperty enabled;

public:
    GtComputeNodeBase(const QString& name, qint32 flags = F_Default);
    ~GtComputeNodeBase();

    void setName(const QString& name);
    const QString& getName() const;

    void setEnabled(bool flag);
    bool isSkippeable() const { return input_node != nullptr; }
    bool isEnabled() const { return enabled; }

    void compute(const cv::Mat* input);
    void setInput(GtComputeNodeBase* node);
    virtual const cv::Mat* getOutput() const;

private:
    friend class GtComputeNodesTreeModel;
    virtual bool onInputChanged(const cv::Mat* input) = 0;
    virtual void update(const cv::Mat* input) = 0;

    void setEnabledAllOutputs();
    void totalMemoryUsage(const GtComputeNodeBase* parent, size_t& result) const;

protected:
    void outputChanged();
    void updateLater();

    virtual size_t getMemoryUsage() const;
};



#endif // GLSLCOMPUTEGRAPH_H
