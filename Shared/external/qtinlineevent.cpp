#include "qtinlineevent.h"
#include <QCoreApplication>

void QtInlineEvent::Post(const QtInlineEvent::Function& function)
{
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEvent(function));
}


