#ifndef PROCCESSFACTORY_H
#define PROCCESSFACTORY_H

#include <string>
#include <atomic>
#include <functional>

#include <SharedModule/internal.hpp>

struct DescProcessValueState
{
    std::wstring Title;
    int Depth;
    bool IsFinished;
    bool IsNextProcessExpected;
    bool IsTitleChanged;

    bool IsShouldStayVisible() const { return !IsFinished || IsNextProcessExpected; }

    DescProcessValueState(const std::wstring& title, int depth, bool isFinished, bool isNextProcessExpected, bool isTitleChanged)
        : Title(title)
        , Depth(depth)
        , IsFinished(isFinished)
        , IsNextProcessExpected(isNextProcessExpected)
        , IsTitleChanged(isTitleChanged)
    {}
};

class _Export ProcessValue
{
protected:
    typedef std::function<void (ProcessValue*)> FCallback;
    ProcessValue(const FCallback& callback);

public:
    virtual ~ProcessValue();

    DescProcessValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsNextProcessExpected(), IsTitleChanged() }; }
    int GetDepth() const { return _valueDepth; }
    const std::wstring& GetTitle() const { return _title; }
    bool IsCanceled() const { return _isCanceled; }
    bool IsNextProcessExpected() const { return _isNextProcessExpected; }
    bool IsFinished() const { return _isFinished; }
    bool IsTitleChanged() const { return _isTitleChanged; }
    virtual class ProcessDeterminateValue* AsDeterminate() { return nullptr; }

protected:
    void setTitle(const std::wstring& title);
    void finish();
    void setNextProcessExpected();

    virtual void incrementStep(int divider);
    void init(const std::wstring& title);

protected:
    friend class ProcessFactory;
    friend class ProcessBase;

    int _valueDepth;
    FCallback _callback;
    std::wstring _title;
    bool _isNextProcessExpected;
    bool _isCanceled;
    bool _isFinished;
    bool _isTitleChanged;
};

struct DescProcessDeterminateValueState : DescProcessValueState
{
    int CurrentStep;
    int StepsCount;

    DescProcessDeterminateValueState(const std::wstring& title, int depth, bool isFinished, bool isNextProcessExpected, int currentStep, int stepsCount, bool isTitleChanged)
        : DescProcessValueState(title, depth, isFinished, isNextProcessExpected, isTitleChanged)
        , CurrentStep(currentStep)
        , StepsCount(stepsCount)
    {}
};

class ProcessDeterminateValue : public ProcessValue
{
    typedef ProcessValue Super;
    using Super::Super;

public:
    ~ProcessDeterminateValue();

    DescProcessDeterminateValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsNextProcessExpected(), GetCurrentStep(), GetStepsCount(), IsTitleChanged() }; }
    int GetCurrentStep() const { return _currentStep; }
    int GetStepsCount() const { return _stepsCount; }
    virtual ProcessDeterminateValue* AsDeterminate() override{ return this; }

private:
    friend class ProcessFactory;
    friend class ProcessBase;

    virtual void incrementStep(int divider) override;

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
