#ifndef IPROGRESS_H
#define IPROGRESS_H

#include <SharedModule/internal.hpp>

class IProgress;

class ProgressIncrementGuard
{
public:
    ProgressIncrementGuard(IProgress* progress) Q_DECL_NOEXCEPT
        : _progress(progress)
    {}

    ~ProgressIncrementGuard();

private:
    IProgress* _progress;
};

class _Export IProgress
{
public:    
    virtual ~IProgress() {}

    virtual void BeginProgress(const QString& title, bool isShadow) = 0;
    virtual void BeginProgress(const QString& title, int count, bool isShadow) = 0;
    virtual void SetProgressTitle(const QString& title) = 0;
    virtual void IncreaseProgressStepsCount(int newCount) = 0;
    virtual void IncrementProgress() = 0;
    virtual bool IsProgressCanceled() const = 0;
};

inline ProgressIncrementGuard::~ProgressIncrementGuard()
{
    _progress->IncrementProgress();
}

#endif // IProgress_H

