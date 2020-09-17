#ifndef PROCCESSFACTORY_H
#define PROCCESSFACTORY_H

#include <string>
#include <atomic>
#include <functional>

struct DescProcessValueState
{
    std::wstring Title;
    int Depth;
    bool IsFinished;
    bool IsCancelable;
    bool IsTitleChanged;

    bool IsShouldStayVisible() const { return !IsFinished; }

    DescProcessValueState(const std::wstring& title, int depth, bool isFinished, bool isCancelable, bool isTitleChanged)
        : Title(title)
        , Depth(depth)
        , IsFinished(isFinished)
        , IsCancelable(isCancelable)
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

    void SetDummy(bool dummy);
    void Cancel();

    DescProcessValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsCancelable(), IsTitleChanged() }; }
    int GetDepth() const { return m_valueDepth; }
    const std::wstring& GetTitle() const { return m_title; }
    bool IsFinished() const { return m_isFinished; }
    bool IsCancelable() const { return m_interruptor != nullptr; }
    bool IsTitleChanged() const { return m_isTitleChanged; }
    virtual class ProcessDeterminateValue* AsDeterminate() { return nullptr; }

protected:
    void setTitle(const std::wstring& title);
    void finish();

    virtual void incrementStep(int divider);
    void init(Interruptor* interruptor, const std::wstring& title);

protected:
    friend class ProcessFactory;
    friend class ProcessBase;

    int m_valueDepth;
    FCallback m_currentCallback;
    FCallback m_callback;
    std::wstring m_title;
    bool m_isFinished;
    Interruptor* m_interruptor;
    bool m_isTitleChanged;
};

struct DescProcessDeterminateValueState : DescProcessValueState
{
    int CurrentStep;
    int StepsCount;

    DescProcessDeterminateValueState(const std::wstring& title, int depth, bool isFinished, bool isCancelable, int currentStep, int stepsCount, bool isTitleChanged)
        : DescProcessValueState(title, depth, isFinished, isCancelable, isTitleChanged)
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

    DescProcessDeterminateValueState GetState() const { return { GetTitle(), GetDepth(), IsFinished(), IsCancelable(), GetCurrentStep(), GetStepsCount(), IsTitleChanged() }; }
    int GetCurrentStep() const { return m_currentStep; }
    int GetStepsCount() const { return m_stepsCount; }
    virtual ProcessDeterminateValue* AsDeterminate() override{ return this; }

private:
    friend class ProcessFactory;
    friend class ProcessBase;

    virtual void incrementStep(int divider) override;

    void init(Interruptor* interruptor, const std::wstring& title, int stepsCount);
    void increaseStepsCount(int value);

private:
    int m_currentStep;
    int m_stepsCount;
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
    ProcessValue::FCallback m_indeterminateOptions;
    ProcessValue::FCallback m_determinateOptions;
    ProcessValue::FCallback m_shadowIndeterminateOptions;
    ProcessValue::FCallback m_shadowDeterminateOptions;
};



#endif // PROCCESSMANAGER_H
