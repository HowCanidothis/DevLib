#include "stateproperty.h"

void StateProperty::SetState(bool state)
{
    Super::SetValue(state);
}

DispatcherConnections StateProperty::ConnectFromStateProperty(const char* location, const StateProperty& property)
{
    return Super::ConnectFrom(location, [this](bool valid) { return valid ? Super::m_value : valid; }, property);
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
        m_isValid = true;
    }
    Q_ASSERT(m_counter >= 0);
}

void StateParameters::Reset()
{
    m_isValid = false;
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
