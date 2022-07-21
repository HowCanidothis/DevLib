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
    DispatcherConnectionsSafe Connection;

    operator qint32() const { return Id; }
};

class LocalPropertyErrorsContainer : public LocalPropertySet<LocalPropertyErrorsContainerValue>
{
    using Super = LocalPropertySet<LocalPropertyErrorsContainerValue>;
public:
    LocalPropertyErrorsContainer();

    void AddError(const Name& errorName, const QString& errorString, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr);
    void AddError(const Name& errorName, const TranslatedStringPtr& errorString, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr);
    void RemoveError(const Name& errorName);
    bool HasError(const Name& errorName) const;

    void Clear();

    QHash<Name, QVariant> ErrorsMetaData;

    DispatcherConnection RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const LocalProperty<bool>& property, bool inverted = false, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr);
    DispatcherConnections RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers, QtMsgType severity = QtMsgType::QtCriticalMsg, const SharedPointer<LocalPropertyBool>& visible = nullptr);
    DispatcherConnections Connect(const QString& prefix, const LocalPropertyErrorsContainer& errors);
    DispatcherConnections ConnectFromError(const Name& errorId, const LocalPropertyErrorsContainer& errors);

    QString ToString() const;
    QStringList ToStringList() const;

    LocalProperty<bool> HasErrors;
    LocalProperty<bool> HasErrorsOrWarnings;
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
