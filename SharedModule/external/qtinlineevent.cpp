#include "qtinlineevent.h"
#include <QCoreApplication>

void QtInlineEvent::Post(const QtInlineEvent::Function& function, qint32 priority)
{
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEvent(function), priority);
}


