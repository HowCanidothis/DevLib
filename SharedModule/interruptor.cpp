#include "interruptor.h"

#include "Process/processbase.h"

void InterruptorData::reset()
{
    m_interupted = false;
}

void InterruptorData::interrupt()
{
    m_interupted = true;
    OnInterrupted.Invoke();
}

void InterruptorData::setParent(InterruptorData* data)
{
    Q_ASSERT(m_parentData == nullptr);
    m_parentData = data;
    data->OnInterrupted += { this, [this]{ interrupt(); } };
}

InterruptorData::InterruptorData()
    : m_interupted(false)
    , m_parentData(nullptr)
{
    OnInterrupted.SetAutoThreadSafe();
}

InterruptorData::~InterruptorData()
{
    if(m_parentData != nullptr) {
        m_parentData->OnInterrupted -= this;
    }
}

Interruptor::Interruptor()
    : m_data(::make_shared<InterruptorData>())
{}

void Interruptor::SetParent(const Interruptor& another) { m_data->setParent(another.m_data.get());  }

void Interruptor::Reset()
{
    m_data->reset();
}

void Interruptor::Interrupt() const { m_data->interrupt(); }

void Interruptor::Interrupt(qint32 msecs) const { auto data = m_data; ThreadTimer::SingleShot(msecs, [data]{ data->interrupt(); }); }

bool Interruptor::IsInterrupted() const { return m_data->m_interupted; }

const Dispatcher& Interruptor::OnInterrupted() const { return m_data->OnInterrupted; }

AsyncRequestPending::AsyncRequestPending(const Name& processId, const QString& label)
    : m_label(label)
    , m_processId(processId)
{

}

AsyncRequestPending::AsyncRequestPending()
{

}

AsyncRequest AsyncRequestPending::SetRequest(const AsyncRequest& request)
{
    m_currentRequest.ResultInterruptor.Interrupt();
    m_currentRequest = request;
    if(!m_label.isEmpty()) {
        auto process = ::make_shared<ProcessBase>();
        process->SetId(m_processId);
        process->BeginProcess(m_label);
        request.Result.Then([process](bool){});
    }
    return request;
}
