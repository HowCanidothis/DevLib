#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QException>

#include "declarations.h"

class Exception;

struct ExceptionHandleParams
{
    using FNotifier = std::function<void (const QString&, const Exception&)>;

    QString ProcessName;
    FAction Action;
    FNotifier Notifier;
    bool StartProcess = false;

    ExceptionHandleParams(const QString& processName, const FAction& action);

    ExceptionHandleParams& RemoveNotificator() { Notifier = [](const QString&, const Exception&){}; return *this; }
    ExceptionHandleParams& SetDefaultNotificatorWarningHandler();
    ExceptionHandleParams& SetDefaultNotificatorHandler(const std::function<void (const QString&, const QString&)>& handlerModuleMessage);
    ExceptionHandleParams& SetNotificator(const FNotifier& notifier);
    ExceptionHandleParams& SetProcessName(const QString& processName) { ProcessName = processName; return *this; }
    ExceptionHandleParams& SetStartProcess(const QString& processName){ StartProcess = true; ProcessName = processName;  return *this; }
};

class Exception : public QException
{
    Q_DECLARE_TR_FUNCTIONS(Exception)
public:
    FTranslationHandler TranslationHandler;
    QVariant ExceptionData;
    Exception(const char* text);
    Exception(const QString& text);
    Exception(const FTranslationHandler& handler);
    Exception(const QVariant& exceptionData);

    void raise() const override;
    QException* clone() const override;

    QString Message() const;

    static bool Handle(const QString& module, const FAction& action);
    static bool Handle(const ExceptionHandleParams& params);
};

#endif // EXCEPTION_H
