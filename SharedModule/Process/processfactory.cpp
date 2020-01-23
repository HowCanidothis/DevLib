#include "processfactory.h"

ProcessValue::ProcessValue(const FCallback& callback)
    : _valueDepth(depthCounter()++)
    , _callback(callback)
    , _isNextProcessExpected(false)
    , _isCanceled(false) 
    , _isFinished(false)
    , _isTitleChanged(false)
{
}

std::atomic_int& ProcessValue::depthCounter()
{
    static std::atomic_int res;
    return res;
}

ProcessValue::~ProcessValue()
{
    finish();
    --depthCounter();
}

void ProcessValue::setTitle(const std::wstring& title)
{
    _title = title;
    _isTitleChanged = true;
    _callback(this);
    _isTitleChanged = false;
}

void ProcessValue::finish()
{
    if(!_isFinished) {
        _isFinished = true;
        _callback(this);
    }
}

void ProcessValue::setNextProcessExpected()
{
    _isNextProcessExpected = true;
}

void ProcessValue::incrementStep(int)
{
}

void ProcessValue::init(const std::wstring& title)
{
    _title = title;
    _isTitleChanged = true;
    _callback(this);
    _isTitleChanged = false;
}

ProcessDeterminateValue::~ProcessDeterminateValue()
{
    finish();
}

void ProcessDeterminateValue::incrementStep(int divider)
{
    _currentStep++;
    if(divider) {
        if(!(_currentStep % divider)) {
            _callback(this);
        }
    } else {
        _callback(this);
    }
}

void ProcessDeterminateValue::init(const std::wstring& title, int stepsCount)
{
    _currentStep = 0;
    _stepsCount = stepsCount;
    Super::init(title);
}

void ProcessDeterminateValue::increaseStepsCount(int value)
{
    _stepsCount += value;
    _callback(this);
}

static bool DoNothingCallback(ProcessValue*) { return true; }

ProcessFactory::ProcessFactory()
    : _indeterminateOptions(&DoNothingCallback)
    , _determinateOptions(&DoNothingCallback)
    , _shadowIndeterminateOptions(&DoNothingCallback)
    , _shadowDeterminateOptions(&DoNothingCallback)
{

}

ProcessFactory& ProcessFactory::Instance()
{
    static ProcessFactory ret;
    return ret;
}

void ProcessFactory::SetDeterminateCallback(const ProcessValue::FCallback& options)
{
    _determinateOptions = options;
}

void ProcessFactory::SetIndeterminateCallback(const ProcessValue::FCallback& options)
{
    _indeterminateOptions = options;
}

void ProcessFactory::SetShadowDeterminateCallback(const ProcessValue::FCallback& options)
{
    _shadowDeterminateOptions = options;
}

void ProcessFactory::SetShadowIndeterminateCallback(const ProcessValue::FCallback& options)
{
    _shadowIndeterminateOptions = options;
}

ProcessValue* ProcessFactory::createIndeterminate() const
{
    return new ProcessValue(_indeterminateOptions);
}

ProcessDeterminateValue* ProcessFactory::createDeterminate() const
{
    return new ProcessDeterminateValue(_determinateOptions);
}

ProcessValue* ProcessFactory::createShadowIndeterminate() const
{
    return new ProcessValue(_shadowIndeterminateOptions);
}

ProcessDeterminateValue*ProcessFactory::createShadowDeterminate() const
{
    return new ProcessDeterminateValue(_shadowDeterminateOptions);
}
