#ifndef PROCCESSBASE_H
#define PROCCESSBASE_H

#include <functional>
#include <memory>

#include "iprocess.h"

class ProcessValue;

class ProcessDummy : public IProcess
{
public:
    virtual void BeginProcess(const wchar_t*, bool) final {}
    virtual void BeginProcess(const wchar_t*, int, bool) final {}
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

    void BeginProcess(const wchar_t* title, bool shadow = false) override;
    void BeginProcess(const wchar_t* title, int stepsCount, bool shadow = false) override;
    void SetProcessTitle(const wchar_t* title) override;
    void IncreaseProcessStepsCount(int stepsCount) override;
    void IncrementProcess() override;
    bool IsProcessCanceled() const override;

private:
    std::unique_ptr<ProcessValue> _processValue;
};


#endif // PROCCESSBASE_H
