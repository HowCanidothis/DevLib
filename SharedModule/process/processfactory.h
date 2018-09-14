#ifndef PROCCESSFACTORY_H
#define PROCCESSFACTORY_H

#include <string>
#include <atomic>
#include <functional>

#include <SharedModule/internal.hpp>

struct DescProccesValueState
{
    std::wstring Title;
    int Depth;
    bool IsFinished;
    bool IsNextProcessExpected;

    bool IsShouldStayVisible() const { return !IsFinished || IsNextProcessExpected; }
};

class _Export ProcessValue
{
    typedef std::function<void (ProcessValue*)> FCallback;
    ProcessValue(const FCallback& callback);

    static std::atomic_int& depthCounter();

public:
    virtual ~ProcessValue();

    DescProccesValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsNextProcessExpected() }; }
    int GetDepth() const { return _valueDepth; }
    const std::wstring& GetTitle() const { return _title; }
    bool IsCanceled() const { return _isCanceled; }
    bool IsNextProcessExpected() const { return _isNextProcessExpected; }
    bool IsFinished() const { return _isFinished; }
    virtual class ProcessDeterminateValue* AsDeterminate() { return nullptr; }

protected:
    void setTitle(const std::wstring& title);
    void finish();
    void setNextProcessExpected();

    virtual void incrementStep();
    void init(const std::wstring& title);

private:
    friend class ProcessFactory;
    friend class ProcessBase;

    int _valueDepth;
    FCallback _callback;
    std::wstring _title;
    bool _isNextProcessExpected;
    bool _isCanceled;
    bool _isFinished;
};

struct DescProcessDeterminateValueState : DescProccesValueState
{
    int CurrentStep;
    int StepsCount;
};

class ProcessDeterminateValue : public ProcessValue
{
    typedef ProcessValue Super;
    using Super::Super;

public:
    ~ProcessDeterminateValue();

    DescProcessDeterminateValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsNextProcessExpected(), GetCurrentStep(), GetStepsCount() }; }
    int GetCurrentStep() const { return _currentStep; }
    int GetStepsCount() const { return _stepsCount; }
    virtual ProcessDeterminateValue* AsDeterminate() override{ return this; }

private:
    virtual void incrementStep() override;

    friend class ProcessBase;
    void init(const std::wstring& title, int stepsCount);
    void increaseStepsCount(int value);

private:
    int _currentStep;
    int _stepsCount;
};

class _Export ProcessFactory
{
    ProcessFactory();
public:
    static ProcessFactory& Instance();

    void SetDeterminateCallback(const ProcessValue::FCallback& options);
    void SetIndeterminateCallback(const ProcessValue::FCallback& options);
    void SetShadowDeterminateCallback(const ProcessValue::FCallback& options);
    void SetShadowIndeterminateCallback(const ProcessValue::FCallback& options);

private:
    friend class ProcessBase;
    ProcessValue* createIndeterminate() const;
    ProcessDeterminateValue* createDeterminate() const;
    ProcessValue* createShadowIndeterminate() const;
    ProcessDeterminateValue* createShadowDeterminate() const;

private:
    ProcessValue::FCallback _indeterminateOptions;
    ProcessValue::FCallback _determinateOptions;
    ProcessValue::FCallback _shadowIndeterminateOptions;
    ProcessValue::FCallback _shadowDeterminateOptions;
};



#endif // PROCCESSMANAGER_H
