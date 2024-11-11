#include "stateproperty.h"

void StateProperty::SetState(bool state)
{
    Super::SetValue(state);
}

DispatcherConnections StateProperty::ConnectFromStateProperty(const char* location, const StateProperty& property)
{
    return Super::ConnectFrom(location, [this](bool valid) { return valid ? Super::m_value : valid; }, property);
}

bool StateProperty::Wait(const char* cdl, const QVector<const StateProperty*>& stateProperties, qint32 msecs)
{
    AsyncResult result;
    OnFirstInvokePerformWhenEveryIsValid(cdl, [&] {
        result.Resolve(true);
    }, stateProperties);
    return result.Wait(msecs);
}

DispatcherConnection StateProperty::OnFirstInvokePerformWhenEveryIsValid(const char* location, const FAction& handler, const QVector<const StateProperty*>& stateProperties)
{
    if(stateProperties.isEmpty()) {
        handler();
        return DispatcherConnection();
    }
    auto commutator = ::make_shared<WithDispatcherConnectionsSafe<LocalPropertyBoolCommutator>>(LocalPropertyBoolCommutator::And);
    for(const auto* prop : stateProperties) {
        commutator->ConnectFrom(location, *prop).MakeSafe(commutator->Connections);
    }

    return commutator->AsProperty().OnChanged.OnFirstInvoke([location, handler, commutator]{
        handler();
        ThreadsBase::DoMain(location, [commutator]{}); // Safe deletion
    });
}

StatePropertyBoolCommutator::StatePropertyBoolCommutator(bool defaultState)
    : Super(And)
{
    m_property.EditSilent() = defaultState;
    m_commutator.OnDirectChanged += { this, [this]{
        if(!value()) {
#ifdef QT_DEBUG
            m_property.SetValueForceInvoke(false);
#else
            m_property.SetValue(false);
#endif
        }
    }};
    adapters::ResetThread(m_commutator.OnDirectChanged);
}

void StatePropertyBoolCommutator::Update()
{
    m_property = false;
    m_commutator.Invoke();
}

void StateParameters::Initialize()
{
    if(m_initializer == nullptr) {
        return;
    }
    m_initializer();
    onInitialized();
    m_initializer = nullptr;
}

IStateParameterBase::IStateParameterBase(StateParameters* params)
{
    params->m_parameters.append(this);
}

StateParameters::StateParameters(bool valid)
    : IsValid(valid)
    , IsLocked(false)
    , m_counter(0)
    , m_isValid(true)
    , m_initializer([this]{
        THREAD_ASSERT_IS_MAIN();
        for(auto* parameter : ::make_const(m_parameters)) {
            if(!parameter->IsInitialized()) {
                parameter->initialize();
            }
        }
    })
{
    DEBUG_SYNC(this, { &IsValid, &m_isValid });
}

void StateParameters::Lock()
{
    ++m_counter;
    IsLocked = m_counter != 0;
}

void StateParameters::Unlock()
{
    --m_counter;
    IsLocked = m_counter != 0;
    if(!IsLocked) {
        m_isValid.SetState(true);
    }
#ifdef QT_DEBUG
    if(m_counter < 0) {
       DEBUG_PRINT_INFO(this);
    }
    Q_ASSERT(m_counter >= 0);
#endif
}

void StateParameters::Reset()
{
    m_isValid.SetState(false);
}

CapturedStateParameters::CapturedStateParameters(bool valid)
    : Super(valid)
    , m_used([this]{
        IsActive = true;
    }, [this]{
        IsActive = false;
    })
{

}

DispatcherConnection DispatcherConnectionChain::OnFailed(const char* cdl, const FAction& action)
{
    if(!IsValid) {
        action();
    }
    return IsValid.OnChanged.Connect(cdl, [this, action] {
        if(!IsValid) {
            action();
        }
    });
}

DispatcherConnection DispatcherConnectionChain::OnValid(const char* cdl, const FAction& action)
{
    if(IsValid) {
        action();
    }
    return IsValid.OnChanged.Connect(cdl, [this, action] {
        if(IsValid) {
            action();
        }
    });
}

DispatcherConnectionChain::DispatcherConnectionChain()
    : IsValid(true)
{

}

const Dispatcher* DispatcherConnectionChain::DefaultDispatcher()
{
    static Dispatcher result;
    return &result;
}

void DispatcherConnectionChain::ConnectFrom(const char* cdl, const DispatcherConnectionChain& another)
{
    connectFrom(cdl, another);
    if(another.IsValid) {
        update();
    } else {
        IsValid.SetState(false);
    }
}

void DispatcherConnectionChain::add(const char* cdl, const FDep& dependency)
{
    m_deps.append(dependency);
}

void DispatcherConnectionChain::invalidResult()
{
    IsValid.SetState(false);
}

void DispatcherConnectionChain::validResult()
{
    IsValid.EditSilent() = false;
    IsValid.SetState(true);
}

void DispatcherConnectionChain::connectFrom(const char* cdl, const DispatcherConnectionChain& another)
{
    add(cdl, [&another] {
        return &another.IsValid.OnChanged;
    });
    add(cdl, [&another]() -> const Dispatcher* {
        return another.IsValid ? DefaultDispatcher() : nullptr;
    });
}

void DispatcherConnectionChain::update()
{
    m_depConnections.clear();
    for(const auto& h : m_deps) {
        auto* disp = h();
        if(disp == nullptr) {
            invalidResult();
            return;
        }
        disp->Connect(CDL, [this]{ update(); }).MakeSafe(m_depConnections);
    }
    validResult();
}
