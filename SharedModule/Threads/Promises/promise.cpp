#include "promise.h"

#include <SharedModule/Threads/threadsbase.h>

AsyncObject::AsyncObject()
{}

AsyncObject::~AsyncObject()
{
}

AsyncResult AsyncObject::Async(const FAction& action, const PromiseData<bool>::FCallback& onDone)
{
    m_result.Mute();
    m_result = ThreadsBase::Async([action]{
        action();
    });
    m_result.Then(onDone);
    return m_result;
}
