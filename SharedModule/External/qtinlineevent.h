#ifndef QTCUSTOMEVENTS_H
#define QTCUSTOMEVENTS_H

#include <QEvent>
#include <functional>

#include "SharedModule/declarations.h"
#include "SharedModule/Threads/Promises/promise.h"

class _Export QtInlineEvent : public QEvent
{
public:
    QtInlineEvent(const char* location, const FAction& function)
        : QEvent(QEvent::User)
        , m_function(function)
#ifdef QT_DEBUG
        , m_location(location)
#endif
    {}
    ~QtInlineEvent() {
        m_function();
    }

    static void Post(const char* location, const FAction& function, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void Post(const char* location, const FAction& function, QObject* object, Qt::EventPriority priority = Qt::NormalEventPriority);

private:
    FAction m_function;
#ifdef QT_DEBUG
    const char* m_location;
#endif
};

class _Export QtInlineEventWithResult : public QtInlineEvent
{
    using Super = QtInlineEvent;
public:
    QtInlineEventWithResult(const char* location, const FAction& function, const AsyncResult& result);

    static AsyncResult Post(const char* location, const FAction& function, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Post(const char* location, const FAction& function, QObject* object, Qt::EventPriority priority = Qt::NormalEventPriority);
};

#endif // QTCUSTOMEVENTS_H
