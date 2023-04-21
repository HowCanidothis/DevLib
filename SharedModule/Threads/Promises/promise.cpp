#include "promise.h"

#include "SharedModule/MemoryManager/memorymanager.h"
#include "SharedModule/smartpointersadapters.h"
#include "SharedModule/Threads/threadsbase.h"
#include "SharedModule/interruptor.h"

PromiseData::PromiseData()
    : m_result(false)
    , m_isResolved(false)
    , m_isCompleted(false)
{}
PromiseData::~PromiseData()
{
    if(!m_isCompleted) {
        resolve(false);
    }
}

void PromiseData::resolve(qint8 value)
{
    resolve([value]{ return value; });
}

void PromiseData::resolve(const std::function<qint8 ()>& handler)
{
    if(m_isResolved) {
        return;
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_isResolved) {
            return;
        }
        m_isResolved = true;
    }
    qint8 value = handler();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_result = value;
        onFinished(value);
        m_isCompleted = true;
    }

    onFinished.Reset();
}

DispatcherConnection PromiseData::then(const FCallback& handler)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_isCompleted) {
        handler(m_result);
        return DispatcherConnection();
    }
    return onFinished.Connect(CONNECTION_DEBUG_LOCATION, handler);
}

void PromiseData::mute()
{
     onFinished.Reset();
}

Promise::Promise()
    : m_data(::make_shared<PromiseData>())
{}

DispatcherConnection Promise::Then(const typename PromiseData::FCallback& handler, const class FutureResult& future) const
{
    AsyncResult tmp;
    future += tmp;
    return m_data->then([future, tmp, handler](qint8 ok){
        handler(ok);
        tmp.Resolve(ok);
    });
}

SafeCall::SafeCall()
    : m_data(::make_shared<SafeCallData>())
{}
SafeCall::~SafeCall()
{
    m_data->ObjectIsDead = true;
}

FAction SafeCall::Wrap(const FAction& handler) const
{
    auto data = m_data;
    return [handler, data]{
        if(!data->ObjectIsDead) {
            handler();
        }
    };
}

qint8 Promise::Wait()
{
    FutureResult result;
    result += *this;
    result.Wait();
    return GetValue();
}

bool Promise::Wait(qint32 msecs)
{
    FutureResult result;
    result += *this;
    return result.Wait(msecs);
}

Promise Promise::MoveToMain(const std::function<qint8 (qint8)>& handler)
{
    Promise promise;
    Then([promise, handler](qint8 res){
        ThreadHandlerMain([promise, handler, res]{
            promise.Resolve(handler(res));
        });
    });
    return promise;
}


AsyncError::AsyncError()
{
    Resolve(false);
}

AsyncSuccess::AsyncSuccess()
{
    Resolve(true);
}

void FutureResultData::ref()
{
    m_promisesCounter++;
}

void FutureResultData::deref()
{
    m_promisesCounter--;

    if(isFinished()) {
        onFinished();
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_conditional.notify_one();
        }
        onFinished.Reset();
    }
}

bool FutureResultData::isFinished() const { return m_promisesCounter == 0; }
qint8 FutureResultData::getResult() const { return m_result; }
void FutureResultData::setResult(qint8 result)
{
    m_promisesCounter = 1;
    m_result = result;
    deref();
}

void FutureResultData::addPromise(const AsyncResult& promise, const SharedPointer<FutureResultData>& self)
{
    ref();
    promise.Then([this, self](const qint8& result){
        m_result |= result;
        deref();
    });

    return;
}

void FutureResultData::then(const std::function<void (qint8)>& action)
{
    if(isFinished()) {
        action(getResult());
    } else {
        onFinished.Connect(CONNECTION_DEBUG_LOCATION, [this, action]{
            action(m_result);
        });
    }
}

void FutureResultData::wait()
{
    if(qApp != nullptr && QThread::currentThread() == qApp->thread()) {
        while(!isFinished()) {
            qApp->processEvents();
        }
    } else {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(!isFinished()) {
            m_conditional.wait(lock);
        }
    }
}

FutureResultData::FutureResultData()
    : m_result(0)
    , m_promisesCounter(0)
{}

FutureResultData::~FutureResultData()
{

}

FutureResult::FutureResult()
    : m_data(::make_shared<FutureResultData>())
{}

bool FutureResult::IsFinished() const { return m_data->isFinished(); }
qint8 FutureResult::GetResult() const { return m_data->getResult(); }
void FutureResult::SetResult(qint8 result) const { m_data->setResult(result); }

void FutureResult::operator+=(const Promise& promise) const
{
    m_data->addPromise(promise, m_data);
}

void FutureResult::Then(const std::function<void (qint8)>& action) const
{
    m_data->then(action);
}

void FutureResult::Wait() const
{
    m_data->wait();
}

bool FutureResult::Wait(qint32 msecs) const
{
    bool result = true;
    Interruptor interruptor;
    auto data = m_data;
    interruptor.OnInterrupted().Connect(CONNECTION_DEBUG_LOCATION, [data, &result]{
        if(data->m_promisesCounter != 0) {
            result = false;
            data->m_promisesCounter = 1;
            data->deref();
        }
    });
    interruptor.Interrupt(msecs);

    m_data->wait();
    return result;
}

AsyncResult FutureResult::ToAsyncResult() const
{
    AsyncResult result;
    Then([result](qint8 done){
        result.Resolve(done);
    });
    return result;
}
