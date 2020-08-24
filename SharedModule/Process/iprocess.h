#ifndef IPROCESS_H
#define IPROCESS_H

#include "SharedModule/namingconvention.h"
#include "SharedModule/shared_decl.h"

class IProcess;

class ProcessIncrementGuard
{
public:
    ProcessIncrementGuard(IProcess* process) Q_DECL_NOEXCEPT
        : m_process(process)
    {

    }
    ~ProcessIncrementGuard();

private:
    IProcess* m_process;
};

class _Export IProcess ATTACH_MEMORY_SPY(IProcess)
{
public:    
    virtual ~IProcess() {}

    virtual void BeginProcess(const wchar_t*, bool) = 0;
    virtual void BeginProcess(const wchar_t*, int, int, bool) = 0;
    virtual void SetProcessTitle(const wchar_t*) = 0;
    virtual void IncreaseProcessStepsCount(int) = 0;
    virtual void IncrementProcess() = 0;
    virtual bool IsProcessCanceled() const = 0;
};

inline ProcessIncrementGuard::~ProcessIncrementGuard()
{
    m_process->IncrementProcess();
}

#endif // IPROCESS_H

