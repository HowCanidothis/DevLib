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
    TranslatedString(const std::function<QString ()>& translationHandler = []{ return QString(); });

    Dispatcher Retranslate;

protected:
    void retranslate();

protected:
    std::function<QString ()> m_translationHandler;
    DispatcherConnectionsSafe m_connections;
    DelayedCallObject m_retranslate;
};

using TranslatedStringPtr = SharedPointer<class TranslatedString>;

#endif // TRANSLATORMANAGER_H
