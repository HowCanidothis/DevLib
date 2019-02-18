#include "qtinlineevent.h"
#include <QCoreApplication>

void QtInlineEvent::Post(const QtInlineEvent::Function& function, Qt::EventPriority priority)
{
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEvent(function), priority);
}

void QtInlineEvent::Post(const QtInlineEvent::Function& function, QObject* object, Qt::EventPriority priority)
{
    QCoreApplication::postEvent(object, new QtInlineEvent(function), priority);
}


