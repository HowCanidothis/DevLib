#ifndef PROCCESSFACTORY_H
#define PROCCESSFACTORY_H

#include "SharedModule/declarations.h"
#include "SharedModule/name.h"
#include "SharedModule/dispatcher.h"

#include <string>
#include <atomic>
#include <functional>

struct DescProcessValueState
{
    Name Id;
    QString Title;
    int Depth;
    bool IsFinished;
    bool IsCancelable;
    bool IsTitleChanged;
    bool IsFromMain;

    bool IsShouldStayVisible() const { return !IsFinished; }

    DescProcessValueState(const Name& id, const QString& title, int depth, bool isFinished, bool isCancelable, bool isTitleChanged, bool isFromMain)
        : Id(id)
        , Title(title)
        , Depth(depth)
        , IsFinished(isFinished)
        , IsCancelable(isCancelable)
        , IsTitleChanged(isTitleChanged)
        , IsFromMain(isFromMain)
    {}
};

struct DescProcessDeterminateValueState : DescProcessValueState
{
    int CurrentStep;
    int StepsCount;

    DescProcessDeterminateValueState(const Name& id, const QString& title, int depth, bool isFinished, bool isCancelable, int currentStep, int stepsCount, bool isTitleChanged, bool isFromMain)
        : DescProcessValueState(id, title, depth, isFinished, isCancelable, isTitleChanged, isFromMain)
        , CurrentStep(currentStep)
        , StepsCount(stepsCount)
    {}

    DescProcessDeterminateValueState()
        : DescProcessValueState(Name(), QString(), -1, false, false, false, false)
        , CurrentStep(0)
        , StepsCount(0)
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

    void SetIsFromMain() { m_isFromMain = true; }

    DescProcessValueState GetState() const { return { GetId(), GetTitle(), GetDepth(), IsFinished(), IsCancelable(), IsTitleChanged(), IsFromMain() }; }
    virtual DescProcessDeterminateValueState GetCommonState() const { return DescProcessDeterminateValueState(GetId(), GetTitle(), GetDepth(), IsFinished(), IsCancelable(), 0, 0, IsTitleChanged(), IsFromMain()); }
    int GetDepth() const { return m_valueDepth; }
    const QString& GetTitle() const { return m_title; }
    void SetId(const Name& id);
    const Name& GetId() const { return m_id; }
    bool IsFinished() const { return m_isFinished; }
    bool IsCancelable() const { return m_interruptor != nullptr; }
    bool IsTitleChanged() const { return m_isTitleChanged; }
    bool IsFromMain() const { return m_isFromMain; }
    virtual class ProcessDeterminateValue* AsDeterminate() { return nullptr; }

protected:
    void setTitle(const QString& title);
    void finish();

    virtual void incrementStep(int divider);
    void init(class Interruptor* interruptor, const QString& title);

protected:
    friend class ProcessFactory;
    friend class ProcessBase;

    int m_valueDepth;
    FCallback m_currentCallback;
    FCallback m_callback;
    QString m_title;
    Name m_id;
    bool m_isFinished;
    Interruptor* m_interruptor;
    bool m_isTitleChanged;
    bool m_isFromMain;
};

class ProcessDeterminateValue : public ProcessValue
{
    typedef ProcessValue Super;
    using Super::Super;

public:
    ~ProcessDeterminateValue();

    DescProcessDeterminateValueState GetState() const { return GetCommonState(); }
    DescProcessDeterminateValueState GetCommonState() const override { return DescProcessDeterminateValueState(GetId(), GetTitle(), GetDepth(), IsFinished(), IsCancelable(), GetCurrentStep(), GetStepsCount(), IsTitleChanged(), IsFromMain()); }
    int GetCurrentStep() const { return m_currentStep; }
    int GetStepsCount() const { return m_stepsCount; }
    virtual ProcessDeterminateValue* AsDeterminate() override{ return this; }

private:
    friend class ProcessFactory;
    friend class ProcessBase;

    virtual void incrementStep(int divider) override;

    void init(Interruptor* interruptor, const QString& title, int stepsCount);
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

    CommonDispatcher<size_t, const DescProcessValueState&> OnIndeterminate;
    CommonDispatcher<size_t, const DescProcessDeterminateValueState&> OnDeterminate;

private:
    friend class ProcessBase;
    ProcessValue* createIndeterminate() const;
    ProcessDeterminateValue* createDeterminate() const;
};



#endif // PROCCESSMANAGER_H
