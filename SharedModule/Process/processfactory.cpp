#include "processfactory.h"
#include "processmanager.h"

ProcessValue::ProcessValue(const FCallback& callback)
    : m_valueDepth(ProcessManager::getInstance().registerNewProcessValue())
    , m_currentCallback(callback)
    , m_callback(callback)
    , m_isFinished(false)
    , m_interruptor(nullptr)
    , m_isTitleChanged(false)
    , m_isFromMain(false)
{
}

void ProcessValue::SetDummy(bool dummy)
{
    if(dummy) {
        m_currentCallback = [](ProcessValue*){};
    } else {
        m_currentCallback = m_callback;
    }
}

ProcessValue::~ProcessValue()
{
    finish();
    ProcessManager::getInstance().unregisterProcessValue(m_valueDepth);
}

void ProcessValue::Cancel()
{
    Q_ASSERT(m_interruptor != nullptr);
    m_interruptor->Interrupt();
}

void ProcessValue::SetId(const Name& id)
{
    m_id = id;
}

void ProcessValue::setTitle(const QString& title)
{
    m_title = title;
    m_isTitleChanged = true;
    m_currentCallback(this);
    m_isTitleChanged = false;
}

void ProcessValue::finish()
{
    if(!m_isFinished) {
        m_isFinished = true;
        m_currentCallback(this);
    }
}

void ProcessValue::incrementStep(int)
{
}

void ProcessValue::init(Interruptor* interruptor, const QString& title)
{
    m_title = title;
    m_isTitleChanged = true;
    m_interruptor = interruptor;
    m_currentCallback(this);
    m_isTitleChanged = false;    
}

ProcessDeterminateValue::~ProcessDeterminateValue()
{
    finish();
}

void ProcessDeterminateValue::incrementStep(int divider)
{
    m_currentStep++;
    if(divider) {
        if(!(m_currentStep % divider)) {
            m_currentCallback(this);
        }
    } else {
        m_currentCallback(this);
    }
}

void ProcessDeterminateValue::init(Interruptor* interruptor, const QString& title, int stepsCount)
{
    m_currentStep = 0;
    m_stepsCount = stepsCount;
    Super::init(interruptor, title);
}

void ProcessDeterminateValue::increaseStepsCount(int value)
{
    m_stepsCount += value;
    m_currentCallback(this);
}

static bool DoNothingCallback(ProcessValue*) { return true; }

ProcessFactory::ProcessFactory()
{

}

ProcessFactory& ProcessFactory::Instance()
{
    static ProcessFactory ret;
    return ret;
}

ProcessValue* ProcessFactory::createIndeterminate() const
{
    return new ProcessValue([this](ProcessValue* value){
        if(QThread::currentThread() == qApp->thread()) {
            value->SetIsFromMain();
        }
        auto state = value->GetState();
        ThreadHandlerMain([value, this, state]{
            OnIndeterminate((size_t)value, state);
        });
    });
}

ProcessDeterminateValue* ProcessFactory::createDeterminate() const
{
    return new ProcessDeterminateValue([this](ProcessValue* value){
        if(QThread::currentThread() == qApp->thread()) {
            value->SetIsFromMain();
        }
        auto state = value->GetCommonState();
        ThreadHandlerMain([value, this, state]{
            OnDeterminate((size_t)value, state);
        });
    });
}
