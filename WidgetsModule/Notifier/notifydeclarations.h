#ifndef NOTIFYDECLARATIONS_H
#define NOTIFYDECLARATIONS_H

#include "WidgetsModule/Models/modelstablebase.h"

using NotifyDataPtr = SharedPointer<class NotifyData>;

class NotifyData {
public:
    enum Columns
    {
        Column_Type,
        Column_Time,
        Column_Body,
    };

    NotifyData(qint32 type, const QString& body)
        : Body(body)
        , Type(type)
        , Time(QTime::currentTime())
    {}

    QString Body;
    qint32 Type;
    QTime Time;

    QVariant GetData(Columns column) const
    {
        switch (column) {
        case Column_Type: return Type;
        case Column_Body: return Body;
        case Column_Time: return Time;
        default: break;
        }
        return QVariant();
    }
};

struct NotifyErrorContainerData
{
    FAction Action;
    class LocalPropertyErrorsContainer* Container;
    Name Id;
    DispatcherConnectionsSafe Connections;

    NotifyErrorContainerData(const FAction& action, LocalPropertyErrorsContainer* container, const Name& id)
        : Action(action)
        , Container(container)
        , Id(id)
    {
    }
};

struct NotifyConsoleData
{
    ScopedPointer<NotifyErrorContainerData> ErrorHandler;
    NotifyDataPtr Data;

    NotifyConsoleData(const NotifyDataPtr& data = nullptr)
        : Data(data)
    {}
};

using NotifyConsoleDataPtr = SharedPointer<NotifyConsoleData>;
using NotifyConsoleDataWrapper = TModelsTableWrapper<QVector<NotifyConsoleDataPtr>>;
using NotifyConsoleDataWrapperPtr = SharedPointer<NotifyConsoleDataWrapper>;

#endif // NOTIFYDECLARATIONS_H