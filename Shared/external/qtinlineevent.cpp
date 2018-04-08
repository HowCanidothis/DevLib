#include "qtinlineevent.h"
#include <QCoreApplication>

void QtInlineEvent::post(const QtInlineEvent::Function& function)
{
    QCoreApplication::postEvent(QCoreApplication::instance(), new QtInlineEvent(function));
}


