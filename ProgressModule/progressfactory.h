#ifndef PROGRESSFACTORY_H
#define PROGRESSFACTORY_H

#include <string>
#include <atomic>
#include <functional>

#include <SharedModule/internal.hpp>

struct DescProgressValueState
{
    QString Title;
    int Depth;
    bool IsFinished;
    bool IsNextProgressExpected;

    bool IsShouldStayVisible() const { return !IsFinished || IsNextProgressExpected; }
};

class _Export ProgressValue
{
    typedef std::function<void (ProgressValue*)> FCallback;
    ProgressValue(const FCallback& callback);

    static std::atomic_int& depthCounter();

public:
    virtual ~ProgressValue();

    DescProgressValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsNextProgressExpected() }; }
    int GetDepth() const { return _valueDepth; }
    const QString& GetTitle() const { return _title; }
    bool IsCanceled() const { return _isCanceled; }
    bool IsNextProgressExpected() const { return _isNextProgressExpected; }
    bool IsFinished() const { return _isFinished; }
    virtual class ProgressDeterminateValue* AsDeterminate() { return nullptr; }

protected:
    void setTitle(const QString& title);
    void finish();
    void setNextProgressExpected();

    virtual void incrementStep();
    void init(const QString& title);

private:
    friend class ProgressFactory;
    friend class Progress;

    int _valueDepth;
    FCallback _callback;
    QString _title;
    bool _isNextProgressExpected;
    bool _isCanceled;
    bool _isFinished;
};

struct DescProgressDeterminateValueState : DescProgressValueState
{
    int CurrentStep;
    int StepsCount;
};

class ProgressDeterminateValue : public ProgressValue
{
    typedef ProgressValue Super;
    using Super::Super;

public:
    ~ProgressDeterminateValue();

    DescProgressDeterminateValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsNextProgressExpected(), GetCurrentStep(), GetStepsCount() }; }
    int GetCurrentStep() const { return _currentStep; }
    int GetStepsCount() const { return _stepsCount; }
    virtual ProgressDeterminateValue* AsDeterminate() override{ return this; }

private:
    virtual void incrementStep() override;

    friend class Progress;
    void init(const QString& title, int stepsCount);
    void increaseStepsCount(int value);

private:
    int _currentStep;
    int _stepsCount;
};

class _Export ProgressFactory
{
    ProgressFactory();
public:
    static ProgressFactory& Instance();

    void SetDeterminateCallback(const ProgressValue::FCallback& options);
    void SetIndeterminateCallback(const ProgressValue::FCallback& options);
    void SetShadowDeterminateCallback(const ProgressValue::FCallback& options);
    void SetShadowIndeterminateCallback(const ProgressValue::FCallback& options);

private:
    friend class Progress;
    ProgressValue* createIndeterminate() const;
    ProgressDeterminateValue* createDeterminate() const;
    ProgressValue* createShadowIndeterminate() const;
    ProgressDeterminateValue* createShadowDeterminate() const;

private:
    ProgressValue::FCallback _indeterminateOptions;
    ProgressValue::FCallback _determinateOptions;
    ProgressValue::FCallback _shadowIndeterminateOptions;
    ProgressValue::FCallback _shadowDeterminateOptions;
};



#endif // PROCCESSMANAGER_H
