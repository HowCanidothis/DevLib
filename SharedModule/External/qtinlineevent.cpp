#include "qtinlineevent.h"
#include <QCoreApplication>

void QtInlineEvent::Post(const FAction& function, Qt::EventPriority priority)
{
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEvent(function), priority);
}

void QtInlineEvent::Post(const FAction& function, QObject* object, Qt::EventPriority priority)
{
    QCoreApplication::postEvent(object, new QtInlineEvent(function), priority);
}

AsyncResult QtInlineEventWithResult::Post(const FAction& function, Qt::EventPriority priority)
{
    AsyncResult result;
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEventWithResult(function, result), priority);
    return result;
}

AsyncResult QtInlineEventWithResult::Post(const FAction& function, QObject* object, Qt::EventPriority priority)
{
    AsyncResult result;
    QCoreApplication::postEvent(object, new QtInlineEventWithResult(function, result), priority);
    return result;
}
