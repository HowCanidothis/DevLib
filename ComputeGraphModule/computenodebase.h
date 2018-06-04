#ifndef COMPUTENODEBASE_H
#define COMPUTENODEBASE_H

#include "SharedModule/array.h"
#include "SharedModule/flags.h"
#include "SharedGuiModule/decl.h"

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

    GtComputeNodeBase* _inputNode;
    Array<GtComputeNodeBase*> _linkedOutputs;
    ScopedPointer<cv::Mat> _output;
    Flags _flags;
    QString _name;
    BoolProperty _enabled;

public:
    GtComputeNodeBase(const QString& name, qint32 flags = F_Default);
    ~GtComputeNodeBase();

    void SetName(const QString& name);
    const QString& GetName() const;

    void SetEnabled(bool flag);
    bool IsSkippeable() const { return _inputNode != nullptr; }
    bool IsEnabled() const { return _enabled; }

    void Compute(const cv::Mat* input);
    void SetInput(GtComputeNodeBase* node);
    virtual const cv::Mat* GetOutput() const;

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
