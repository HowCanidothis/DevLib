#ifndef QTCUSTOMEVENTS_H
#define QTCUSTOMEVENTS_H

#include <QEvent>
#include <functional>

#include "SharedModule/shared_decl.h"

class _Export QtInlineEvent : public QEvent
{
public:
    typedef std::function<void ()> Function;
    QtInlineEvent(const Function& function)
        : QEvent(QEvent::User)
        , _function(function)
    {}
    ~QtInlineEvent() {
        _function();
    }

    static void Post(const Function& function, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void Post(const Function& function, QObject* object, Qt::EventPriority priority = Qt::NormalEventPriority);
private:
    Function _function;
};

#endif // QTCUSTOMEVENTS_H
