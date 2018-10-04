#ifndef PROGRESS_H
#define PROGRESS_H

#include <functional>
#include <memory>
#include "iprogress.h"

class ProgressValue;

class ProgressDummy : public IProgress
{
public:
    virtual void BeginProgress(const QString&, bool) final {}
    virtual void BeginProgress(const QString&, int, bool) final {}
    virtual void SetProgressTitle(const QString&) final {}
    virtual void IncreaseProgressStepsCount(int) final {}
    virtual void IncrementProgress() final {}
    virtual bool IsProgressCanceled() const final { return false; }
};

class _Export Progress : public IProgress
{
    typedef std::function<void ()> FOnFinish;

public:
    Progress();
    ~Progress();

    void BeginProgress(const QString& title, bool shadow = false) Q_DECL_OVERRIDE;
    void BeginProgress(const QString& title, int stepsCount, bool shadow = false) Q_DECL_OVERRIDE;
    void SetProgressTitle(const QString& title) Q_DECL_OVERRIDE;
    void IncreaseProgressStepsCount(int stepsCount) Q_DECL_OVERRIDE;
    void IncrementProgress() Q_DECL_OVERRIDE;
    bool IsProgressCanceled() const Q_DECL_OVERRIDE;

private:
    ScopedPointer<ProgressValue> _progressValue;
};


#endif // PROCCESSBASE_H
