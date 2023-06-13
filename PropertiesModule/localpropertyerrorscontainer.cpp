#include "localpropertyerrorscontainer.h"

#include "translatormanager.h"

LocalPropertyErrorsContainer::LocalPropertyErrorsContainer()
    : HasErrors(false)
    , HasErrorsOrWarnings(false)
    , OnErrorsLabelsChanged(500)
{
    OnChanged += {this, [this]{
        HasErrorsOrWarnings = !IsEmpty();
        for(const auto& value : *this) {
            if(value.Type == QtMsgType::QtCriticalMsg || value.Type == QtMsgType::QtFatalMsg) {
                HasErrors = true;
                return;
            }
        }
        HasErrors = false;
    }};

    TranslatorManager::GetInstance().OnLanguageChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        OnErrorsLabelsChanged();
    }).MakeSafe(m_connections);
}

void LocalPropertyErrorsContainer::AddError(const Name& errorName, const QString& errorString, QtMsgType severity, const SharedPointer<LocalPropertyBool>& visible)
{
    AddError(errorName, ::make_shared<TranslatedString>([errorString]{ return errorString; }), severity, visible);
}

void LocalPropertyErrorsContainer::AddError(const Name& errorName, const TranslatedStringPtr& errorString, QtMsgType severity, const SharedPointer<LocalPropertyBool>& visible, const FAction& focus)
{
    LocalPropertyErrorsContainerValue toInsert{ errorName, errorString, severity, visible, focus };
    if(Super::Native().contains(toInsert)) {
       return;
    }
    toInsert.Connection = OnErrorsLabelsChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, errorString->OnChanged).MakeSafe();
    if(Super::Insert(toInsert)) {
        OnErrorAdded(toInsert);
    }
}

bool LocalPropertyErrorsContainer::HasError(const Name& errorName) const
{
    LocalPropertyErrorsContainerValue toRemove{ errorName };
    return Super::IsContains(toRemove);
}

const LocalPropertyErrorsContainerValue* LocalPropertyErrorsContainer::GetError(const Name& errorName) const
{
    LocalPropertyErrorsContainerValue toRemove{ errorName };
    auto iter = Super::Find(toRemove);
    return iter == Super::end() ? nullptr : &(*iter);
}

void LocalPropertyErrorsContainer::RemoveError(const Name& errorName)
{
    LocalPropertyErrorsContainerValue toRemove{ errorName };
    if(Super::Remove(toRemove)) {
        OnErrorRemoved(toRemove);
    }
}

DispatcherConnections LocalPropertyErrorsContainer::RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const LocalPropertyBool& property, bool inverted, QtMsgType severity, const SharedPointer<LocalPropertyBool>& visible, const FAction& focus)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_registeredErrors.contains(errorId));
    m_registeredErrors.insert(errorId);
#endif
    return property.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this, errorId, errorString, inverted, severity, visible, focus](bool value){
        if(value ^ inverted) {
            AddError(errorId, errorString, severity, visible, focus);
        } else {
            RemoveError(errorId);
        }
    });
}

DispatcherConnections LocalPropertyErrorsContainer::RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers, QtMsgType severity, const SharedPointer<LocalPropertyBool>& visible, const FAction& focus)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_registeredErrors.contains(errorId));
    m_registeredErrors.insert(errorId);
#endif
    DispatcherConnections result;
    auto update = [this, validator, errorId, errorString, severity, visible, focus]{
        if(!validator()) {
            AddError(errorId, errorString, severity, visible, focus);
        } else {
            RemoveError(errorId);
        }
    };

    for(auto* dispatcher : dispatchers) {
        result += dispatcher->Connect(CONNECTION_DEBUG_LOCATION, update);
    }

    update();
    return result;
}

void LocalPropertyErrorsContainer::Clear()
{
    for(const auto& error : *this) {
        OnErrorRemoved(error);
    }
    Super::Clear();
}

DispatcherConnections LocalPropertyErrorsContainer::Connect(const QString& prefix, const LocalPropertyErrorsContainer& errors)
{
    auto* pErrors = const_cast<LocalPropertyErrorsContainer*>(&errors);
    auto addError = [this, prefix](const LocalPropertyErrorsContainerValue& value){
        AddError(Name(prefix + value.Id.AsString()), value.Error);
    };
    auto removeError = [this, prefix](const LocalPropertyErrorsContainerValue& value) {
        RemoveError(Name(prefix + value.Id.AsString()));
    };
    DispatcherConnections result;
    result += pErrors->OnErrorAdded.Connect(CONNECTION_DEBUG_LOCATION, addError);
    result += pErrors->OnErrorRemoved.Connect(CONNECTION_DEBUG_LOCATION, removeError);
    for(const auto& error : errors) {
        AddError(Name(prefix + error.Id.AsString()), error.Error);
    }
    return result;
}

DispatcherConnections LocalPropertyErrorsContainer::ConnectFromError(const Name& errorId, const LocalPropertyErrorsContainer& errors)
{
    auto* pErrors = const_cast<LocalPropertyErrorsContainer*>(&errors);
    auto addError = [this, errorId](const LocalPropertyErrorsContainerValue& value){
        if(errorId == value.Id) {
            AddError(errorId, value.Error);
        }
    };
    auto removeError = [this, errorId](const LocalPropertyErrorsContainerValue& value) {
        if(errorId == value.Id) {
            RemoveError(errorId);
        }
    };
    DispatcherConnections result;
    result += pErrors->OnErrorAdded.Connect(CONNECTION_DEBUG_LOCATION, addError);
    result += pErrors->OnErrorRemoved.Connect(CONNECTION_DEBUG_LOCATION, removeError);
    auto foundIt = errors.Native().find(LocalPropertyErrorsContainerValue{ errorId });
    if(foundIt != errors.end()) {
        AddError(errorId, foundIt->Error);
    }
    return result;
}

DispatcherConnections LocalPropertyErrorsContainer::ConnectFromErrors(const char* debugLocation, const LocalPropertyErrorsContainer& errors, const QSet<Name>& activeErrors)
{
    return ConnectFrom(debugLocation, [activeErrors](const QSet<LocalPropertyErrorsContainerValue>& errors){
        QSet<LocalPropertyErrorsContainerValue> result;
        for(const auto& errorId : activeErrors){
            auto iter = errors.find({errorId});
            if(iter != errors.end()){
                result.insert(*iter);
            }
        }
        return result;
    }, errors);
}

QString LocalPropertyErrorsContainer::ToString() const
{
    QString resultText;
    for(const auto& error : *this) {
        resultText += error.Error->Native() + "\n";
    }
    return resultText;
}

QStringList LocalPropertyErrorsContainer::ToStringList() const
{
    QStringList result;
    for(const auto& error : *this) {
        result += error.Error->Native();
    }
    return result;
}
