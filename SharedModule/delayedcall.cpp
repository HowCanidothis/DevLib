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
    m_result = result;
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

AsyncResult DelayedCallManager::CallDelayed(DelayedCallObject* object, const FAction& action, const ThreadHandlerNoThreadCheck& handler)
{
    QMutexLocker locker(mutex());
    auto foundIt = cachedCalls().find(object);
    if(foundIt == cachedCalls().end()) {
        auto delayedCall = ::make_shared<DelayedCall>(action, mutex(), object);
        cachedCalls().insert(object, delayedCall);
        auto result = handler([delayedCall]{
            delayedCall->Call();
        });
        result.Then([object](bool){
            QMutexLocker locker(mutex());
            cachedCalls().remove(object);
        });
        delayedCall->SetResult(result);
        return result;
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
        }, threadHandler);
    };

    for(auto* dispatcher : dispatchers) {
        dispatcher->Connect(this, callOnChanged); // Note. eternal subscribe
    }
}
