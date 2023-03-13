#ifndef TRANSLATORMANAGER_H
#define TRANSLATORMANAGER_H

#include "localproperty.h"
#include "localpropertydeclarations.h"

class TranslatorManager
{
    TranslatorManager();
public:
    static TranslatorManager& GetInstance();

    template <typename Enum>
    static bool IsValid(int value) { return static_cast<int>(Enum::First) <= value && value <= static_cast<int>(Enum::Last); }
    template <typename Enum>
    static bool IsValid(Enum value) { return IsValid<Enum>( static_cast<int>(value)); }

    template<typename Enum>
    static bool SetValueFromString(qint32& valueToChange, const QString& value)
    {
        return SetValueFromString((Enum&)valueToChange, value);
    }

    template<typename Enum>
    static bool SetValueFromString(Enum& valueToChange, const QString& value)
    {
        auto setter = SetterFromString(valueToChange, value);
        if(setter != nullptr) {
            setter();
            return true;
        }
        return false;
    }

    template<typename Enum>
    static QVariant ToVariant(qint32 value)
    {
        return IsValid<Enum>(value) ? QVariant(GetNames<Enum>().at(value)) : QVariant();
    }

    template<typename Enum>
    static QVariant ToVariant(Enum value)
    {
        return ToString<Enum>(static_cast<int>(value));
    }

    template<typename Enum>
    static QString ToString(qint32 value)
    {
        return IsValid<Enum>(value) ? GetNames<Enum>().at(value) : "";
    }

    template<typename Enum>
    static QString ToString(Enum value)
    {
        return ToString<Enum>(static_cast<int>(value));
    }

    template<typename Enum>
    static FAction SetterFromString(Enum& valueToChange, const QString& value)
    {
        int index = GetNames<Enum>().indexOf(value);
        if(!IsValid<Enum>(index)){
            return nullptr;
        }
        LambdaValueWrapper<Enum&> wrapper(valueToChange);
        return [wrapper, index]{ wrapper.GetValue() = (Enum)index; };
    }

    template<typename Enum>
    static FAction SetterFromString(qint32& valueToChange, const QString& value)
    {
        return SetterFromString((Enum&)valueToChange, value);
    }

    template<class T>
    QStringList GetEnumNames()
    {
        if(QThread::currentThread() != QCoreApplication::instance()->thread()) {
            return EnumHelper<T>::GetNames();
        }

        Name name(typeid (T).name());
        auto foundIt = m_names.find(name);
        if(foundIt == m_names.end()) {
            foundIt = m_names.insert(name, EnumHelper<T>::GetNames());
        }
        return foundIt.value();
    }

    template<class T>
    static QStringList GetNames()
    {
        return TranslatorManager::GetInstance().GetEnumNames<T>();
    }

    Dispatcher OnLanguageChanged;

private:
    QHash<Name, QStringList> m_names;
};

template<typename Enum>
inline QStringList LocalPropertySequentialEnum<Enum>::GetNames() const
{
    return TranslatorManager::GetInstance().GetEnumNames<Enum>();
}

class TranslatedString : public LocalPropertyString
{
    using Super = LocalPropertyString;
public:
    TranslatedString(const FTranslationHandler& translationHandler = TR_NONE);
    TranslatedString(const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& retranslators);

    template<typename ... Dispatchers>
    void SetTranslationHandler(const FTranslationHandler& handler, Dispatchers&... dispatchers)
    {
        auto connections = DispatcherConnectionsSafeCreate();
        SetTranslationHandler(TR(handler(), handler, connections));
        Retranslate.ConnectFrom(CONNECTION_DEBUG_LOCATION, dispatchers...).MakeSafe(*connections);
    }
    void SetTranslationHandler(const FTranslationHandler& handler);
    DispatcherConnections SetTranslationHandler(const char* connectionInfo, const FTranslationHandler& handler, const QVector<Dispatcher*>& retranslators);
    const FTranslationHandler& GetTranslationHandler() const { return m_translationHandler; }

    Dispatcher Retranslate;
    DispatcherConnectionsSafe Connections;

protected:
    void retranslate();

protected:
    FTranslationHandler m_translationHandler;
    DelayedCallObject m_retranslate;
};

Q_DECLARE_METATYPE(SharedPointer<TranslatedString>)

#define TRANSLATED_PTR(handler, translators, ...) \
    ::make_shared<TranslatedString>([__VA_ARGS__]{ return handler; }, translators)

#define TRANSLATED_PTR_1(handler, ...) \
    ::make_shared<TranslatedString>([__VA_ARGS__]{ return handler; })

#define TRS(handler, ...) TRANSLATED_PTR_1(handler, __VA_ARGS__)
#define TR_ENUM(value) TranslatorManager::GetInstance().ToString(value)
#endif // TRANSLATORMANAGER_H
