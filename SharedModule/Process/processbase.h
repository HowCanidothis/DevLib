#ifndef PROCCESSBASE_H
#define PROCCESSBASE_H

#include <functional>
#include <memory>

#include "iprocess.h"
#include "SharedModule/interruptor.h"

class ProcessValue;

class ProcessDummy : public IProcess
{
public:
    virtual void BeginProcess(const wchar_t*, bool) final {}
    virtual void BeginProcess(const wchar_t*, int, int, bool) final {}
    virtual void SetProcessTitle(const wchar_t*) final {}
    virtual void IncreaseProcessStepsCount(int) final {}
    virtual void IncrementProcess() final {}
    virtual bool IsProcessCanceled() const final { return false; }
};

class _Export ProcessBase : public IProcess
{
    typedef std::function<void ()> FOnFinish;

public:
    ProcessBase();
    ~ProcessBase();

    void SetInterruptor(const Interruptor& interruptor);
    void SetSilentIfOneStep(bool silentIfOneStep) { m_silentIfOneStep = silentIfOneStep; }
    const std::wstring& GetTitle() const;

    void BeginProcess(const wchar_t* title, bool shadow = false) override;
    void BeginProcess(const wchar_t* title, int stepsCount, int wantedCount, bool shadow = false) override;
    void SetProcessTitle(const wchar_t* title) override;
    void IncreaseProcessStepsCount(int stepsCount) override;
    void IncrementProcess() override;
    bool IsProcessCanceled() const override;

private:
    std::unique_ptr<ProcessValue> m_processValue;
    int m_divider;
    ScopedPointer<Interruptor> m_interruptor;
    bool m_silentIfOneStep;
};


#endif // PROCCESSBASE_H
