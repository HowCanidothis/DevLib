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
        result.append(dispatcher->Connect(this, [this, delayedCall]{
            SetState(false);
            delayedCall->Call([this]{
                SetState(true);
            });
        }));
    }
    return result;
}

DispatcherConnections StateProperty::PerformWhenEveryIsValid(const QVector<LocalPropertyBool*>& stateProperties, const FAction& handler, qint32 delayMsecs, bool once)
{
    auto commutator = ::make_shared<LocalPropertyBoolCommutator>(true, delayMsecs);
    auto connections = ::make_shared<DispatcherConnections>(commutator->AddProperties(CONNECTION_DEBUG_LOCATION, stateProperties));
    commutator->Update();
    *connections += commutator->OnChanged.ConnectAndCall(nullptr, [connections, handler, commutator, once]{
        if(*commutator) {
            handler();
            if(once) {
                ThreadsBase::DoMain([connections]{
                    for(auto& connection : *connections) {
                        connection.Disconnect();
                    }
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
            m_setTrue.Call([this]{
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
    SetValue(false);
    m_commutator.Invoke();
}

DispatcherConnections StatePropertyBoolCommutator::AddProperties(const char* location, const QVector<LocalProperty<bool>*>& properties)
{
    DispatcherConnections result;
    for(auto* property : properties) {
        if(*property == !m_defaultState) {
            SetValue(false);
        }
        result += m_commutator.ConnectFrom(location, property->OnChanged);
    }
    m_properties += properties;
    return result;
}

QString StatePropertyBoolCommutator::ToString() const
{
    QString result;
    for(auto* property : m_properties) {
        result += *property ? "true " : "false ";
    }
    return result;
}

bool StatePropertyBoolCommutator::value() const
{
    bool result = m_defaultState;
    bool oppositeState = !result;
    for(auto* property : m_properties) {
        if(*property == oppositeState) {
            result = oppositeState;
            break;
        }
    }
    return result;
}

StateParameters::StateParameters()
    : IsValid(true)
    , m_counter(0)
    , m_isValid(true)
{
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
