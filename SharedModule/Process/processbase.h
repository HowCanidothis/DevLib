#ifndef PROCCESSBASE_H
#define PROCCESSBASE_H

#include <functional>
#include <memory>

#include "SharedModule/interruptor.h"

class ProcessValue;

struct ProcessBaseIndeterminateParams
{
    bool Shadow = false;
    QString Title;

    ProcessBaseIndeterminateParams(const QString& title)
        : Title(title)
    {}

    ProcessBaseIndeterminateParams& SetShadow() { Shadow = true; return *this; }
};


struct ProcessBaseDeterminateParams : ProcessBaseIndeterminateParams
{
    using Super = ProcessBaseIndeterminateParams;

    qint32 WantedCount = 100;
    qint32 StepsCount;

    ProcessBaseDeterminateParams(const QString& title)
        : Super(title)
    {}

    ProcessBaseDeterminateParams& SetLimits(qint32 stepsCount, qint32 wantedCount = 100)
    {
        StepsCount = stepsCount;
        WantedCount = wantedCount;
        return *this;
    }
};

class _Export ProcessBase
{
public:
    ProcessBase();
    ~ProcessBase();

    void SetInterruptor(const Interruptor& interruptor);
    void SetSilentIfOneStep(bool silentIfOneStep) { m_silentIfOneStep = silentIfOneStep; }
    const QString& GetTitle() const;

    void IncreaseProcessStepsCount(int stepsCount);
    void IncrementProcess();
    bool IsProcessCanceled() const;
    void BeginProcess(const ProcessBaseIndeterminateParams& params);
    void BeginProcess(const ProcessBaseDeterminateParams& params);
    void SetProcessTitle(const QString&title);

    void BeginShadowProcess(const QString& title)
    {
        BeginProcess(ProcessBaseIndeterminateParams(title).SetShadow());
    }
    void BeginShadowProcess(const QString& title, int stepsCount, int wantedCount = 100)
    {
        BeginProcess(ProcessBaseDeterminateParams(title).SetLimits(stepsCount, wantedCount).SetShadow());
    }

private:
    std::unique_ptr<ProcessValue> m_processValue;
    int m_divider;
    ScopedPointer<Interruptor> m_interruptor;
    bool m_silentIfOneStep;
};

class ProcessIncrementGuard
{
public:
    ProcessIncrementGuard(ProcessBase* process) Q_DECL_NOEXCEPT
        : m_process(process)
    {

    }
    ~ProcessIncrementGuard()
    {
        m_process->IncrementProcess();
    }

private:
    ProcessBase* m_process;
};

using QProcessBase = ProcessBase;

#endif // PROCCESSBASE_H
