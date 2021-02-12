#include "delayedcall.h"

DelayedCall::DelayedCall(const FAction& action, QMutex* mutex, DelayedCallObject* object)
    : m_action(action)
    , m_mutex(mutex)
{
    m_connection = object->OnDeleted.Connect(nullptr, [this]{
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

QHash<void*, DelayedCallPtr>& DelayedCallManager::cachedCalls()
{
    static QHash<void*, DelayedCallPtr> result;
    return result;
}

AsyncResult DelayedCallManager::CallDelayed(DelayedCallObject* object, const FAction& action)
{
    QMutexLocker locker(mutex());
    auto foundIt = cachedCalls().find(object);
    if(foundIt == cachedCalls().end()) {
        auto delayedCall = object->m_delay != 0 ? ::make_shared<DelayedCallDelayOnCall>(action, mutex(), object) :
                                                                                   ::make_shared<DelayedCall>(action, mutex(), object);
        foundIt = cachedCalls().insert(object, delayedCall);
        auto result = delayedCall->Invoke(object->m_threadHandler, [delayedCall]{
            delayedCall->Call();
        }, object->m_delay);
        delayedCall->GetResult().Then([object](bool){
            QMutexLocker locker(mutex());
            cachedCalls().remove(object);
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

DelayedCallDispatchersCommutator::DelayedCallDispatchersCommutator()
{

}

void DelayedCallDispatchersCommutator::Subscribe(const ThreadHandlerNoThreadCheck& threadHandler, const QVector<Dispatcher*>& dispatchers)
{
    auto callOnChanged = [this, threadHandler]{
        DelayedCallManager::CallDelayed(&m_delayedCallObject, [this]{
            Invoke();
        });
    };

    for(auto* dispatcher : dispatchers) {
        dispatcher->Connect(this, callOnChanged); // Note. eternal subscribe
    }
}
