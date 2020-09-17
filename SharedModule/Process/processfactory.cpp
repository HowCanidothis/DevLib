#include "processfactory.h"
#include "processmanager.h"

ProcessValue::ProcessValue(const FCallback& callback)
    : m_valueDepth(ProcessManager::getInstance().registerNewProcessValue())
    , m_currentCallback(callback)
    , m_callback(callback)
    , m_isFinished(false)
    , m_interruptor(nullptr)
    , m_isTitleChanged(false)
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

void ProcessValue::setTitle(const std::wstring& title)
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

void ProcessValue::init(Interruptor* interruptor, const std::wstring& title)
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

void ProcessDeterminateValue::init(Interruptor* interruptor, const std::wstring& title, int stepsCount)
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
    : m_indeterminateOptions(&DoNothingCallback)
    , m_determinateOptions(&DoNothingCallback)
    , m_shadowIndeterminateOptions(&DoNothingCallback)
    , m_shadowDeterminateOptions(&DoNothingCallback)
{

}

ProcessFactory& ProcessFactory::Instance()
{
    static ProcessFactory ret;
    return ret;
}

void ProcessFactory::SetDeterminateCallback(const ProcessValue::FCallback& options)
{
    m_determinateOptions = options;
}

void ProcessFactory::SetIndeterminateCallback(const ProcessValue::FCallback& options)
{
    m_indeterminateOptions = options;
}

void ProcessFactory::SetShadowDeterminateCallback(const ProcessValue::FCallback& options)
{
    m_shadowDeterminateOptions = options;
}

void ProcessFactory::SetShadowIndeterminateCallback(const ProcessValue::FCallback& options)
{
    m_shadowIndeterminateOptions = options;
}

ProcessValue* ProcessFactory::createIndeterminate() const
{
    return new ProcessValue(m_indeterminateOptions);
}

ProcessDeterminateValue* ProcessFactory::createDeterminate() const
{
    return new ProcessDeterminateValue(m_determinateOptions);
}

ProcessValue* ProcessFactory::createShadowIndeterminate() const
{
    return new ProcessValue(m_shadowIndeterminateOptions);
}

ProcessDeterminateValue*ProcessFactory::createShadowDeterminate() const
{
    return new ProcessDeterminateValue(m_shadowDeterminateOptions);
}
