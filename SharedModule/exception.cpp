#include "exception.h"

static QString DefaultProcessName("Calculations error");

Exception::Exception(const QString& text)
    : Exception([text]{ return text; })
{}
Exception::Exception(const FTranslationHandler& handler)
    : TranslationHandler(handler)
    , Type(-1)
{}
Exception::Exception(qint32 type)
    : Type(type)
{}

void Exception::raise() const { throw *this; }
QException* Exception::clone() const { return new Exception(TranslationHandler); }

QString Exception::Message() const { return TranslationHandler == nullptr ? QString() : TranslationHandler(); }

ExceptionHandleParams& ExceptionHandleParams::SetNotificator(const ExceptionHandleParams::FNotifier& notifier)
{
    if(notifier != nullptr) {
        Notifier = notifier;
    }
    return *this;
}

bool Exception::Handle(const QString& module, const FAction& action)
{
    return Handle(ExceptionHandleParams(action).SetProcessName(module));
}
bool Exception::Handle(const ExceptionHandleParams& params)
{
    ScopedPointer<QProcessBase> process;
    try {
        if(params.StartProcess) {
            process = new QProcessBase();
            process->BeginShadowProcess(params.ProcessName);
        }
        params.Action();
        return true;
    } catch(const Exception& exception) {
        params.Notifier(params.ProcessName, exception);
    } catch(...) {
        params.Notifier(params.ProcessName, Exception([params]{ return tr("Unhandled exception"); }));
    }
    return false;
}

ExceptionHandleParams::ExceptionHandleParams(const FAction& action)
    : ProcessName(DefaultProcessName)
    , Action(action)
    , Notifier([](const QString& module, const Exception& ex){ qCCritical(LC_UI) << QString("%1: %2").arg(module, ex.Message()); })
{}
