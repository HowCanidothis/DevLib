#ifndef LOCALPROPERTYERRORSCONTAINER_H
#define LOCALPROPERTYERRORSCONTAINER_H

#include "localproperty.h"
#include "localpropertydeclarations.h"

struct LocalPropertyErrorsContainerValue
{
    Name Id;
    TranslatedStringPtr Error;

    operator qint32() const { return Id; }
};

class LocalPropertyErrorsContainer : public LocalPropertySet<LocalPropertyErrorsContainerValue>
{
    using Super = LocalPropertySet<LocalPropertyErrorsContainerValue>;
public:
    LocalPropertyErrorsContainer();

    void AddError(const Name& errorName, const QString& errorString);
    void AddError(const Name& errorName, const TranslatedStringPtr& errorString);
    void RemoveError(const Name& errorName);

    DispatcherConnection RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const LocalProperty<bool>& property, bool inverted = false);
    DispatcherConnections RegisterError(const Name& errorId, const TranslatedStringPtr& errorString, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers);
    DispatcherConnections Connect(const QString& prefix, const LocalPropertyErrorsContainer& errors);

    QString ToString() const;
    QStringList ToStringList() const;

    LocalProperty<bool> HasErrors;
    DelayedCallDispatcher OnErrorsLabelsChanged;
    CommonDispatcher<const LocalPropertyErrorsContainerValue&> OnErrorAdded;
    CommonDispatcher<const LocalPropertyErrorsContainerValue&> OnErrorRemoved;

private:
#ifdef QT_DEBUG
    QSet<Name> m_registeredErrors;
#endif
    DispatcherConnectionsSafe m_connections;
};

#endif // LOCALPROPERTYERRORSCONTAINER_H
