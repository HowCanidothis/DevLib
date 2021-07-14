#include "translatormanager.h"

TranslatorManager& TranslatorManager::GetInstance()
{
    static TranslatorManager result;
    return result;
}

TranslatedString::TranslatedString()
{

}

TranslatedString::TranslatedString(const FTranslationHandler& translationHandler)
    : Super(translationHandler())
    , m_translationHandler(translationHandler)
{
    TranslatorManager::GetInstance().OnLanguageChanged.Connect(this, [this, translationHandler]{
        retranslate();
    }).MakeSafe(m_connections);

    Retranslate += { this, [this]{ retranslate(); }};
}

void TranslatedString::SetTranslationHandler(const FTranslationHandler& handler)
{
    m_translationHandler = handler;
    retranslate();
}

void TranslatedString::retranslate()
{
    m_retranslate.Call([this]{
        SetValue(m_translationHandler());
    });
}
