#include "localpropertyerrorscontainer.h"

#include "translatormanager.h"

LocalPropertyErrorsContainer::LocalPropertyErrorsContainer()
    : HasErrors(false)
    , OnErrorsLabelsChanged(500)
{
    OnChange += {this, [this]{
        HasErrorsOrWarnings = !IsEmpty();
        for(const auto& value : *this) {
            if(value.Type == QtMsgType::QtCriticalMsg || value.Type == QtMsgType::QtFatalMsg) {
                HasErrors = true;
                return;
            }
        }
        HasErrors = false;
    }};

    TranslatorManager::GetInstance().OnLanguageChanged.Connect(this, [this]{
        OnErrorsLabelsChanged();
    }).MakeSafe(m_connections);
}

void LocalPropertyErrorsContainer::AddError(const Name& errorName, const QString& errorString, QtMsgType severity)
{
    AddError(errorName, ::make_shared<TranslatedString>([errorString]{ return errorString; }), severity);
}

void LocalPropertyErrorsContainer::AddError(const Name& errorName, const TranslatedStringPtr& errorString, QtMsgType severity)
{
    LocalPropertyErrorsContainerValue toInsert{ errorName, errorString, severity };
    if(Super::Native().contains(toInsert)) {
       return;
    }
    toInsert.Connection = OnErrorsLabelsChanged.ConnectFrom(errorString->OnChange).MakeSafe();
    if(Super::Insert(toInsert)) {
        OnErrorAdded(toInsert);
    }
}

bool LocalPropertyErrorsContainer::HasError(const Name& errorName) const
{
    LocalPropertyErrorsContainerValue toRemove{ errorName, ::make_shared<TranslatedString>([]{ return QString(); }) };
    return Super::IsContains(toRemove);
}

void LocalPropertyErrorsContainer::RemoveError(const Name& errorName)
{
    LocalPropertyErrorsContainerValue toRemove{ errorName, ::make_shared<TranslatedString>([]{ return QString(); }) };
    if(Super::Remove(toRemove)) {
        OnErrorRemoved(toRemove);
    }
}

DispatcherConnection LocalPropertyErrorsContainer::RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const LocalProperty<bool>& property, bool inverted, QtMsgType severity)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_registeredErrors.contains(errorId));
    m_registeredErrors.insert(errorId);
#endif
    auto* pProperty = const_cast<LocalProperty<bool>*>(&property);
    auto update = [this, errorId, pProperty, errorString, inverted, severity]{
        if(*pProperty ^ inverted) {
            AddError(errorId, errorString, severity);
        } else {
            RemoveError(errorId);
        }
    };
    update();
    return pProperty->OnChange.Connect(this, update);
}

DispatcherConnections LocalPropertyErrorsContainer::RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers, QtMsgType severity)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_registeredErrors.contains(errorId));
    m_registeredErrors.insert(errorId);
#endif
    DispatcherConnections result;
    auto update = [this, validator, errorId, errorString, severity]{
        if(!validator()) {
            AddError(errorId, errorString, severity);
        } else {
            RemoveError(errorId);
        }
    };

    for(auto* dispatcher : dispatchers) {
        result += dispatcher->Connect(this, update);
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
    result += pErrors->OnErrorAdded.Connect(this, addError);
    result += pErrors->OnErrorRemoved.Connect(this, removeError);
    for(const auto& error : errors) {
        AddError(Name(prefix + error.Id.AsString()), error.Error);
    }
    return result;
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
