#include "translatormanager.h"

TranslatorManager::TranslatorManager()
{
    OnLanguageChanged += { this, [this]{
        m_names.clear();
    } };
    OnLanguageChanged.SetAutoThreadSafe();
}

TranslatorManager& TranslatorManager::GetInstance()
{
    static TranslatorManager result;
    return result;
}

TranslatedString::TranslatedString(const FTranslationHandler& translationHandler)
    : Super(translationHandler())
    , m_translationHandler(translationHandler)
{
    TranslatorManager::GetInstance().OnLanguageChanged.Connect(CONNECTION_DEBUG_LOCATION, [this, translationHandler]{
        retranslate();
    }).MakeSafe(Connections);

    Retranslate += { this, [this]{ retranslate(); }};
}

TranslatedString::TranslatedString(const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& retranslators)
    : TranslatedString(translationHandler)
{
    for(auto* retranslator : retranslators) {
        Retranslate.ConnectFrom(CONNECTION_DEBUG_LOCATION, *retranslator).MakeSafe(Connections);
    }
}

void TranslatedString::SetTranslationHandler(const FTranslationHandler& handler)
{
    m_translationHandler = handler;
    retranslate();
}

DispatcherConnections TranslatedString::SetTranslationHandler(const char* connectionInfo, const FTranslationHandler& handler, const QVector<Dispatcher*>& retranslators)
{
    DispatcherConnections result;
    m_translationHandler = handler;
    for(auto* dispatcher : retranslators) {
        result += Retranslate.ConnectFrom(connectionInfo, *dispatcher);
    }
    retranslate();
    return result;
}

void TranslatedString::retranslate()
{
	Q_ASSERT(m_translationHandler != nullptr);
    m_retranslate.Call(CONNECTION_DEBUG_LOCATION, [this]{
        SetValue(m_translationHandler());
    });
}
