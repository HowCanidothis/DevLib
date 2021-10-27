#ifndef TRANSLATORMANAGER_H
#define TRANSLATORMANAGER_H

#include "localproperty.h"
#include "localpropertydeclarations.h"

class TranslatorManager
{
    TranslatorManager();
public:
    static TranslatorManager& GetInstance();

    template<class T>
    const QStringList& GetEnumNames()
    {
        THREAD_ASSERT_IS_MAIN();
        Name name(typeid (T).name());
        auto foundIt = m_names.find(name);
        if(foundIt == m_names.end()) {
            foundIt = m_names.insert(name, EnumHelper<T>::GetNames());
        }
        return foundIt.value();
    }

    template<class T>
    static const QStringList& GetNames()
    {
        return TranslatorManager::GetInstance().GetEnumNames<T>();
    }

    Dispatcher OnLanguageChanged;

private:
    QHash<Name, QStringList> m_names;
};

template<typename Enum>
inline const QStringList& LocalPropertySequentialEnum<Enum>::GetNames() const
{
    return TranslatorManager::GetInstance().GetEnumNames<Enum>();
}

template<typename Enum>
inline QStringList LocalPropertySequentialEnum<Enum>::GetNamesThreadSafe() const
{
    if(QThread::currentThread() == qApp->thread()) {
        return TranslatorManager::GetInstance().GetEnumNames<Enum>();
    }
    QStringList result;
    ThreadsBase::DoMainAwait([&result]{
        result = TranslatorManager::GetInstance().GetEnumNames<Enum>()
    });
    return result;
}

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

#define TRANSLATED_PTR(handler, translators) \
    ::make_shared<TranslatedString>([this]{ return handler; }, translators)

#define TRANSLATED_PTR_1(handler) \
    ::make_shared<TranslatedString>([]{ return handler; })

#endif // TRANSLATORMANAGER_H
