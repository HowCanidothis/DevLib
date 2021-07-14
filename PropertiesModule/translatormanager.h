#ifndef TRANSLATORMANAGER_H
#define TRANSLATORMANAGER_H

#include "localproperty.h"

class TranslatorManager
{
    TranslatorManager(){}
public:
    static TranslatorManager& GetInstance();

    Dispatcher OnLanguageChanged;
};

class TranslatedString : public LocalPropertyString
{
    using Super = LocalPropertyString;
    TranslatedString();
public:
    using FTranslationHandler = std::function<QString ()>;

    TranslatedString(const FTranslationHandler& translationHandler = []{ return QString(); });

    void SetTranslationHandler(const FTranslationHandler& handler);

    Dispatcher Retranslate;

protected:
    void retranslate();

protected:
    FTranslationHandler m_translationHandler;
    DispatcherConnectionsSafe m_connections;
    DelayedCallObject m_retranslate;
};

using TranslatedStringPtr = SharedPointer<class TranslatedString>;

#endif // TRANSLATORMANAGER_H
