#include "delayedcall.h"

#include <QUuid>

#include "SharedModule/Threads/threadtimer.h"

DelayedCallObject::DelayedCallObject(qint32 delayMsecs, const ThreadHandlerNoThreadCheck& handler)
    : m_threadHandler(handler)
    , m_delay(delayMsecs)
    , m_id(generateId())
{
}

qint32 DelayedCallObject::generateId()
{
    static qint32 id = 0;
    return id++;
}

DelayedCall::DelayedCall(const FAction& action, QMutex* mutex, DelayedCallObject* object)
    : m_action(action)
    , m_mutex(mutex)
{
    m_connection = object->OnDeleted.Connect(this, [this]{
        QMutexLocker locker(m_mutex);
        m_action = []{};
    }).MakeSafe();
}

void DelayedCall::SetAction(const FAction &action)
{
    // QMutexLocker locker(m_mutex);
    m_action = action;
}

void DelayedCall::SetResult(const AsyncResult& result)
{
    m_resultConnection = result.Then([this](bool value){
        m_result.Resolve(value);
    }).MakeSafe();
}

AsyncResult DelayedCall::Invoke(const ThreadHandlerNoThreadCheck& threadHandler, const FAction& delayedCall, qint32)
{
    return threadHandler(delayedCall);
}

void DelayedCall::Call()
{
    FAction action;
    {
        QMutexLocker locker(m_mutex);
        action = m_action;
    }
    action();
}

DelayedCallDelayOnCall::DelayedCallDelayOnCall(const FAction& action, QMutex* mutex, DelayedCallObject* object)
    : Super(action, mutex, object)
    , m_counter(0)
{

}

void DelayedCallDelayOnCall::Call()
{
    if(--m_counter == 0) {
        Super::Call();
    }
}

AsyncResult DelayedCallDelayOnCall::Invoke(const ThreadHandlerNoThreadCheck& threadHandler, const FAction& delayedCall, qint32 delay)
{
    m_counter++;
    AsyncResult result;
    ThreadTimer::SingleShot(delay, [threadHandler, delayedCall, result]{
        threadHandler(delayedCall).Then([result](bool value){
            result.Resolve(value);
        });
    });
    return result;
}


QMutex* DelayedCallManager::mutex()
{
    static QMutex result;
    return &result;
}

QHash<qint32, DelayedCallPtr>& DelayedCallManager::cachedCalls()
{
    static QHash<qint32, DelayedCallPtr> result;
    return result;
}

AsyncResult DelayedCallManager::CallDelayed(DelayedCallObject* object, const FAction& action)
{
    static thread_local bool locked = false;
    if(locked) {
        return AsyncError();
    }
    QMutexLocker locker(mutex());
    guards::LambdaGuard guard([]{ locked = false; }, []{ locked = true; });
    auto foundIt = cachedCalls().find(object->GetId());
    if(foundIt == cachedCalls().end()) {
        auto delayedCall = object->m_delay != 0 ? ::make_shared<DelayedCallDelayOnCall>(action, mutex(), object) :
                                                                                   ::make_shared<DelayedCall>(action, mutex(), object);
        foundIt = cachedCalls().insert(object->GetId(), delayedCall);
        auto result = delayedCall->Invoke(object->m_threadHandler, [delayedCall]{
            delayedCall->Call();
        }, object->m_delay);
        qint32 id = object->GetId();
        delayedCall->GetResult().Then([id](bool){
            if(locked) {
                return;
            }
            QMutexLocker locker(mutex());
            cachedCalls().remove(id);
        });
        delayedCall->SetResult(result);
        return delayedCall->GetResult();
    } else if(object->m_delay != 0) {
        auto delayedCall = foundIt.value();
        auto result = delayedCall->Invoke(object->m_threadHandler, [delayedCall]{
            delayedCall->Call();
        }, object->m_delay);
        delayedCall->SetResult(result);
    }
    foundIt.value()->SetAction(action);
    return foundIt.value()->GetResult();
}

DelayedCallDispatchersCommutator::DelayedCallDispatchersCommutator(qint32 msecs, const ThreadHandlerNoThreadCheck& threadHandler)
    : m_delayedCallObject(msecs, threadHandler)
{

}

DispatcherConnections DelayedCallDispatchersCommutator::Subscribe(const QVector<CommonDispatcher<>*>& dispatchers)
{
    auto callOnChanged = [this]{
        DelayedCallManager::CallDelayed(&m_delayedCallObject, [this]{
            Invoke();
        });
    };

    DispatcherConnections result;
    for(auto* dispatcher : dispatchers) {
        result += dispatcher->Connect(this, callOnChanged); // Note. eternal subscribe
    }
    return result;
}
