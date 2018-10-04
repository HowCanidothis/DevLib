#include "progress.h"
#include "progressfactory.h"


Progress::Progress()
{

}

Progress::~Progress()
{

}

void Progress::BeginProgress(const QString& title, bool shadow)
{
    if(_progressValue != nullptr) {
        _progressValue->setNextProgressExpected();
    }
    _progressValue = nullptr;
    _progressValue = shadow ? ProgressFactory::Instance().createShadowIndeterminate() : ProgressFactory::Instance().createIndeterminate();
    _progressValue->init(title);
}

void Progress::BeginProgress(const QString& title, int stepsCount, bool shadow)
{
    if(_progressValue != nullptr) {
        _progressValue->setNextProgressExpected();
    }
    _progressValue = nullptr;
    auto value = shadow ? ProgressFactory::Instance().createShadowDeterminate() : ProgressFactory::Instance().createDeterminate();
    value->init(title, stepsCount);
    _progressValue = value;
}

void Progress::SetProgressTitle(const QString& title)
{
    _progressValue->setTitle(title);
}

void Progress::IncreaseProgressStepsCount(int stepsCount)
{
    if(auto determinate = _progressValue->AsDeterminate()) {
        determinate->increaseStepsCount(stepsCount);
    }
}

void Progress::IncrementProgress()
{
    _progressValue->incrementStep();
}

bool Progress::IsProgressCanceled() const
{
    return _progressValue->IsCanceled();
}
