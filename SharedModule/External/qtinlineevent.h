#ifndef QTCUSTOMEVENTS_H
#define QTCUSTOMEVENTS_H

#include <QEvent>
#include <functional>

#include "SharedModule/shared_decl.h"
#include "SharedModule/Threads/Promises/promise.h"

class _Export QtInlineEvent : public QEvent
{
public:
    QtInlineEvent(const FAction& function)
        : QEvent(QEvent::User)
        , m_function(function)
    {}
    ~QtInlineEvent() {
        m_function();
    }

    static void Post(const FAction& function, Qt::EventPriority priority = Qt::NormalEventPriority);
    static void Post(const FAction& function, QObject* object, Qt::EventPriority priority = Qt::NormalEventPriority);

private:
    FAction m_function;
};

class _Export QtInlineEventWithResult : public QtInlineEvent
{
    using Super = QtInlineEvent;
public:
    QtInlineEventWithResult(const FAction& function, const AsyncResult& result)
        : Super([this, function]{
            function();
            m_asyncResult->Resolve(true);
            delete m_asyncResult;
        })
        , m_asyncResult(new AsyncResult(result))
    {}

    ~QtInlineEventWithResult()
    {

    }

    static AsyncResult Post(const FAction& function, Qt::EventPriority priority = Qt::NormalEventPriority);
    static AsyncResult Post(const FAction& function, QObject* object, Qt::EventPriority priority = Qt::NormalEventPriority);

private:
    AsyncResult* m_asyncResult;
};

#endif // QTCUSTOMEVENTS_H
