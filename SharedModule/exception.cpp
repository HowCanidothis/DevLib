#include "exception.h"

static QString DefaultProcessName("Calculations error");

Exception::Exception(const QString& text)
    : Exception([text]{ return text; })
{}
Exception::Exception(const FTranslationHandler& handler)
    : TranslationHandler(handler)
{}
Exception::Exception(const QVariant& data)
    : ExceptionData(data)
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
    return Handle(ExceptionHandleParams(module, action));
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
        auto notifier = params.Notifier;
        auto processName = params.ProcessName;
        notifier(processName, exception);
    } catch(...) {
        auto notifier = params.Notifier;
        auto processName = params.ProcessName;
        notifier(processName, Exception([]{ return tr("Unhandled exception"); }));
    }
    return false;
}

ExceptionHandleParams::ExceptionHandleParams(const QString& processName, const FAction& action)
    : ProcessName(processName)
    , Action(action)
{
    SetDefaultNotificatorHandler([](const QString& module, const QString& message){
        qCCritical(LC_UI) << QString("%1: %2").arg(module, message);
    });
}

ExceptionHandleParams& ExceptionHandleParams::SetDefaultNotificatorHandler(const std::function<void (const QString&, const QString&)>& handlerModuleMessage)
{
    Notifier = [handlerModuleMessage](const QString& module, const Exception& ex){
        DelayedCallManager::CallDelayed(CONNECTION_DEBUG_LOCATION, Name(module), [ex, module, handlerModuleMessage]{
            auto message = ex.Message();
            if(message.isEmpty()) {
                return;
            }
            handlerModuleMessage(module, message);
        }, DelayedCallObjectParams(1000));
    };
    return *this;
}

ExceptionHandleParams& ExceptionHandleParams::SetDefaultNotificatorWarningHandler()
{
    SetDefaultNotificatorHandler([](const QString& module, const QString& message){
        qCWarning(LC_UI) << QString("%1: %2").arg(module, message);
    });
    return *this;
}
