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
    : Super(defaultState)
    , m_defaultState(defaultState)
{
    m_commutator += { this, [this]{
        if(value()) {
            m_setTrue.Call(CONNECTION_DEBUG_LOCATION, [this]{
                SetValue(value());
            });
        } else {
#ifdef QT_DEBUG
            SetValueForceInvoke(false);
#else
            SetValue(false);
#endif
        }
    }};
}

void StatePropertyBoolCommutator::ClearProperties()
{
    m_properties.clear();
}

void StatePropertyBoolCommutator::Update()
{
    SetValue(!m_defaultState);
    m_commutator.Invoke();
}

DispatcherConnections StatePropertyBoolCommutator::AddProperties(const char* location, const QVector<LocalPropertyBool*>& properties)
{
    auto handler = [properties, location, this] {
        for(auto* property : properties) {
            if(*property == !m_defaultState) {
                DEBUG_PRINT_INFO(property);
                return !m_defaultState;
            }
        }
        return m_defaultState;
    };
    QVector<Dispatcher*> dispatchers;
    for(auto* property : properties) {
        dispatchers.append(&property->OnChanged);
    }

    return AddHandlerFromDispatchers(location, handler, dispatchers);
}

DispatcherConnections StatePropertyBoolCommutator::AddProperty(const char* location, LocalPropertyBool* property, bool inverted)
{
    return AddHandler(location, [property, inverted]() -> bool { return inverted ? !property->Native() : property->Native(); }, *property);
}

DispatcherConnections StatePropertyBoolCommutator::AddHandlerFromDispatchers(const char* location, const FHandler& handler, const QVector<Dispatcher*>& dispatchers)
{
    DispatcherConnections result;
    for(auto* dispatcher : dispatchers) {
        result += m_commutator.ConnectFrom(location, *dispatcher);
    }
    m_properties += handler;
    m_commutator.Invoke();
    return result;
}

QString StatePropertyBoolCommutator::ToString() const
{
    QString result;
    for(const auto& handler : m_properties) {
        result += handler() ? "true " : "false ";
    }
    return result;
}

bool StatePropertyBoolCommutator::value() const
{
    bool result = m_defaultState;
    bool oppositeState = !result;
    for(const auto& handler : m_properties) {
        if(handler() == oppositeState) {
            result = oppositeState;
            break;
        }
    }
    return result;
}

void StateParameters::Initialize()
{
    if(m_initializer == nullptr) {
        return;
    }
    m_initializer();
    m_initializer = nullptr;
}

IStateParameterBase::IStateParameterBase(StateParameters* params)
{
    params->m_parameters.append(this);
}

StateParameters::StateParameters()
    : IsValid(true)
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
