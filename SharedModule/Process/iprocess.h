#ifndef IPROCESS_H
#define IPROCESS_H

#include "SharedModule/namingconvention.h"
#include "SharedModule/shared_decl.h"

class IProcess;

class ProcessIncrementGuard
{
public:
    ProcessIncrementGuard(IProcess* process) Q_DECL_NOEXCEPT
        : _process(process)
    {

    }
    ~ProcessIncrementGuard();

private:
    IProcess* _process;
};

class _Export IProcess
{
public:    
    virtual ~IProcess() {}

    virtual void BeginProcess(const wchar_t*, bool) = 0;
    virtual void BeginProcess(const wchar_t*, int, bool) = 0;
    virtual void SetProcessTitle(const wchar_t*) = 0;
    virtual void IncreaseProcessStepsCount(int) = 0;
    virtual void IncrementProcess() = 0;
    virtual bool IsProcessCanceled() const = 0;
};

inline ProcessIncrementGuard::~ProcessIncrementGuard()
{
    _process->IncrementProcess();
}

#endif // IPROCESS_H

