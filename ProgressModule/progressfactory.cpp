#include "Progressfactory.h"

ProgressValue::ProgressValue(const FCallback& callback)
    : _valueDepth(depthCounter()++)
    , _callback(callback)
    , _isCanceled(false)
    , _isFinished(false)
    , _isNextProgressExpected(false)
{
}

std::atomic_int& ProgressValue::depthCounter()
{
    static std::atomic_int res;
    return res;
}

ProgressValue::~ProgressValue()
{
    finish();
    --depthCounter();
}

void ProgressValue::setTitle(const QString& title)
{
    _title = title;
    _callback(this);
}

void ProgressValue::finish()
{
    if(!_isFinished) {
        _isFinished = true;
        _callback(this);
    }
}

void ProgressValue::setNextProgressExpected()
{
    _isNextProgressExpected = true;
}

void ProgressValue::incrementStep()
{
    _callback(this);
}

void ProgressValue::init(const QString& title)
{
    _title = title;
    _callback(this);
}

ProgressDeterminateValue::~ProgressDeterminateValue()
{
    finish();
}

void ProgressDeterminateValue::incrementStep()
{
    _currentStep++;
    Super::incrementStep();
}

void ProgressDeterminateValue::init(const QString& title, int stepsCount)
{
    _currentStep = 0;
    _stepsCount = stepsCount;
    Super::init(title);
}

void ProgressDeterminateValue::increaseStepsCount(int value)
{
    _stepsCount += value;
    (void)Super::incrementStep();
}

static bool DoNothingCallback(ProgressValue*) { return true; }

ProgressFactory::ProgressFactory()
    : _indeterminateOptions(&DoNothingCallback)
    , _determinateOptions(&DoNothingCallback)
    , _shadowIndeterminateOptions(&DoNothingCallback)
    , _shadowDeterminateOptions(&DoNothingCallback)
{

}

ProgressFactory& ProgressFactory::Instance()
{
    static ProgressFactory ret;
    return ret;
}

void ProgressFactory::SetDeterminateCallback(const ProgressValue::FCallback& options)
{
    _determinateOptions = options;
}

void ProgressFactory::SetIndeterminateCallback(const ProgressValue::FCallback& options)
{
    _indeterminateOptions = options;
}

void ProgressFactory::SetShadowDeterminateCallback(const ProgressValue::FCallback& options)
{
    _shadowDeterminateOptions = options;
}

void ProgressFactory::SetShadowIndeterminateCallback(const ProgressValue::FCallback& options)
{
    _shadowIndeterminateOptions = options;
}

ProgressValue* ProgressFactory::createIndeterminate() const
{
    return new ProgressValue(_indeterminateOptions);
}

ProgressDeterminateValue* ProgressFactory::createDeterminate() const
{
    return new ProgressDeterminateValue(_determinateOptions);
}

ProgressValue* ProgressFactory::createShadowIndeterminate() const
{
    return new ProgressValue(_shadowIndeterminateOptions);
}

ProgressDeterminateValue* ProgressFactory::createShadowDeterminate() const
{
    return new ProgressDeterminateValue(_shadowDeterminateOptions);
}
