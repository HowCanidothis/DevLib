#include "translatormanager.h"

TranslatorManager& TranslatorManager::GetInstance()
{
    static TranslatorManager result;
    return result;
}

TranslatedString::TranslatedString()
{

}

TranslatedString::TranslatedString(const std::function<QString ()>& translationHandler)
    : Super(translationHandler())
    , m_translationHandler(translationHandler)
{
    TranslatorManager::GetInstance().OnLanguageChanged.Connect(this, [this, translationHandler]{
        retranslate();
    }).MakeSafe(m_connections);
}

void TranslatedString::retranslate()
{
    m_retranslate.Call([this]{
        SetValue(m_translationHandler());
    });
}
