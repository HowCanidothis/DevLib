#ifndef LOCALPROPERTYERRORSCONTAINER_H
#define LOCALPROPERTYERRORSCONTAINER_H

#include "localproperty.h"
#include "localpropertydeclarations.h"

struct LocalPropertyErrorsContainerValue
{
    Name Id;
    TranslatedStringPtr Error;
    QtMsgType Type = QtMsgType::QtCriticalMsg;
    SharedPointer<LocalPropertyBool> Visible;
    FAction FocusHandler;
    DispatcherConnectionsSafe Connection;

    operator qint32() const { return Id; }
};

class LocalPropertyErrorsContainer : public LocalPropertySet<LocalPropertyErrorsContainerValue>
{
    using Super = LocalPropertySet<LocalPropertyErrorsContainerValue>;
public:
    LocalPropertyErrorsContainer();

    void AddError(const Name& errorName, const QString& errorString, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr);
    void AddError(const Name& errorName, const TranslatedStringPtr& errorString, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr, const FAction& focus = nullptr);
    void RemoveError(const Name& errorName);
    bool HasError(const Name& errorName) const;
    const LocalPropertyErrorsContainerValue* GetError(const Name& errorName) const;

    void Clear();

    QHash<Name, QVariant> ErrorsMetaData;

    DispatcherConnections RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const LocalPropertyBool& property, bool inverted = false, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr, const FAction& focus = nullptr);
    DispatcherConnections RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr, const FAction& focus = nullptr);
    template<typename Function, typename ... Args>
    DispatcherConnections RegisterCritical(const Name& errorId, const TranslatedStringPtr& errorString, const Function& validator, Args ... args) {
        return RegisterError(QtCriticalMsg, errorId, errorString, validator, args...);
    }
    template<typename Function, typename ... Args>
    DispatcherConnections RegisterWarning(const Name& errorId, const TranslatedStringPtr& errorString, const Function& validator, Args ... args) {
        return RegisterError(QtWarningMsg, errorId, errorString, validator, args...);
    }
    template<typename Function, typename ... Args>
    DispatcherConnections RegisterError(QtMsgType severity, const Name& errorId, const TranslatedStringPtr& errorString, const Function& validator, Args ... args) {
#ifdef QT_DEBUG
        Q_ASSERT(!m_registeredErrors.contains(errorId));
        m_registeredErrors.insert(errorId);
#endif
        DispatcherConnections result;
        auto update = [this, validator, errorId, errorString, severity, args...]{
            if(!validator(args->Native()...)) {
                AddError(errorId, errorString, severity);
            } else {
                RemoveError(errorId);
            }
        };

        adapters::Combine([&](const auto property){
            result += property->ConnectAction(CONNECTION_DEBUG_LOCATION, update);
        }, args...);
        update();
        return result;
    }
    DispatcherConnections Connect(const QString& prefix, const LocalPropertyErrorsContainer& errors);
    DispatcherConnections ConnectFromError(const Name& errorId, const LocalPropertyErrorsContainer& errors);
    DispatcherConnections ConnectFromErrors(const char* debugLocation, const LocalPropertyErrorsContainer& errors, const QSet<Name>& activeErrors);

    QString ToString() const;
    QString ToErrorString(const QString& separator = "\n") const;
    QStringList ToStringList() const;

    LocalPropertyBool HasErrors;
    LocalPropertyBool HasErrorsOrWarnings;
    DispatchersCommutator OnErrorsLabelsChanged;
    CommonDispatcher<const LocalPropertyErrorsContainerValue&> OnErrorAdded;
    CommonDispatcher<const LocalPropertyErrorsContainerValue&> OnErrorRemoved;

private:
#ifdef QT_DEBUG
    QSet<Name> m_registeredErrors;
#endif
    DispatcherConnectionsSafe m_connections;
};

#endif // LOCALPROPERTYERRORSCONTAINER_H
