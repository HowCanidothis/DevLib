#include "stateproperty.h"

void StateProperty::SetState(bool state)
{
    Super::SetValue(state);
}

DispatcherConnections StateProperty::ConnectFromStateProperty(const char* location, const StateProperty& property)
{
    return Super::ConnectFrom(location, property, [this](bool valid) { return valid ? Super::m_value : valid; });
}

DispatcherConnections StateProperty::ConnectFromDispatchers(const QVector<Dispatcher*>& dispatchers, qint32 delayMsecs)
{
    auto delayedCall = ::make_shared<DelayedCallObject>(delayMsecs);
    DispatcherConnections result;
    for(auto* dispatcher : dispatchers) {
        result += dispatcher->Connect(CONNECTION_DEBUG_LOCATION, [this, delayedCall]{
            SetState(false);
            delayedCall->Call(CONNECTION_DEBUG_LOCATION, [this]{
                SetState(true);
            });
        });
    }
    return result;
}

DispatcherConnections StateProperty::PerformWhenEveryIsValid(const QVector<LocalPropertyBool*>& stateProperties, const FAction& handler, qint32 delayMsecs, bool once)
{
    auto commutator = ::make_shared<LocalPropertyBoolCommutator>(true, delayMsecs);
    auto connections = ::make_shared<DispatcherConnections>(commutator->AddProperties(CONNECTION_DEBUG_LOCATION, stateProperties));
    commutator->Update();
    *connections += commutator->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [connections, handler, commutator, once]{
        if(*commutator) {
            handler();
            if(once) {
                ThreadsBase::DoMain([connections]{
                    connections->Disconnect();
                });
            }
        }
    });
    return *connections;
}

DispatcherConnections StateProperty::OnFirstInvokePerformWhenEveryIsValid(const QVector<LocalPropertyBool*>& stateProperties, const FAction& handler)
{
    return PerformWhenEveryIsValid(stateProperties, handler, 0, true);
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
            SetValue(false);
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

DispatcherConnections StatePropertyBoolCommutator::AddProperties(const char* location, const QVector<LocalProperty<bool>*>& properties)
{
    auto handler = [properties, location, this] {
        for(auto* property : properties) {
            if(*property == !m_defaultState) {
#ifdef CALCUTION_DEBUG
                qDebug() << location << " " << (property->Description != nullptr ? property->Description() : "")<< *property;
#endif
                return !m_defaultState;
            }
        }
        return m_defaultState;
    };
    QVector<Dispatcher*> dispatchers;
    for(auto* property : properties) {
        dispatchers.append(&property->OnChanged);
    }

    return AddHandler(location, handler, dispatchers);
}

DispatcherConnections StatePropertyBoolCommutator::AddProperty(const char* location, LocalProperty<bool>* property, bool inverted)
{
    return AddHandler(location, [property, inverted]() -> bool { return inverted ? !property->Native() : property->Native(); }, { &property->OnChanged });
}

DispatcherConnections StatePropertyBoolCommutator::AddHandler(const char* location, const FHandler& handler, const QVector<Dispatcher*>& dispatchers)
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

StateParameters::StateParameters(const std::function<QString()>& description)
    : IsValid(true)
    , IsLocked(false)
    , m_counter(0)
    , m_isValid(true)
{
#ifdef CALCUTION_DEBUG
    Description = description;
    IsValid.Description = description;
    m_isValid.Description = description;
#endif
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
