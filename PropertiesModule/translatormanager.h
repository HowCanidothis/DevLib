#ifndef TRANSLATORMANAGER_H
#define TRANSLATORMANAGER_H

#include "localproperty.h"
#include "localpropertydeclarations.h"

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
    TranslatedString(const FTranslationHandler& translationHandler = []{ return QString(); });
    TranslatedString(const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& retranslators);

    void SetTranslationHandler(const FTranslationHandler& handler);

    Dispatcher Retranslate;

protected:
    void retranslate();

protected:
    FTranslationHandler m_translationHandler;
    DispatcherConnectionsSafe m_connections;
    DelayedCallObject m_retranslate;
};

#endif // TRANSLATORMANAGER_H
