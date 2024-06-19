#include "qtinlineevent.h"
#include <QCoreApplication>

void QtInlineEvent::Post(const char* location, const FAction& function, Qt::EventPriority priority)
{
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEvent(location, function), priority);
}

void QtInlineEvent::Post(const char* location, const FAction& function, QObject* object, Qt::EventPriority priority)
{
    QCoreApplication::postEvent(object, new QtInlineEvent(location, function), priority);
}

QtInlineEventWithResult::QtInlineEventWithResult(const char* location, const FAction& function, const AsyncResult& result)
    : Super(location, [location, function, result]{
        result.Resolve([location, function]{
            try {
                function();
                return true;
            } catch (...) {
                return false;
            }
        });
    })
{}

AsyncResult QtInlineEventWithResult::Post(const char* location, const FAction& function, Qt::EventPriority priority)
{
    AsyncResult result;
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEventWithResult(location, function, result), priority);
    return result;
}

AsyncResult QtInlineEventWithResult::Post(const char* location, const FAction& function, QObject* object, Qt::EventPriority priority)
{
    AsyncResult result;
    QCoreApplication::postEvent(object, new QtInlineEventWithResult(location, function, result), priority);
    return result;
}
