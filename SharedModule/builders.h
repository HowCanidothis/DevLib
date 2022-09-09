#ifndef BUILDERS_H
#define BUILDERS_H

namespace lq
{

template<class Container>
class Iterators : public QVector<typename Container::const_iterator>
{
    using Super = QVector<typename Container::const_iterator>;
public:

    friend QDebug operator<<(QDebug out, const Iterators<Container>& target)
    {
        out << "IteratorsValue[";
        for(const auto& it : target) {
            out.maybeSpace() << *it;
        }
        out << "]";
        return out;
    }
};

template<class T>
struct AdapterHelper
{
    using Type = T;
    template<typename Iterator = typename T::const_iterator>
    static const Iterator& Extract(const Iterator& value) { return value; }
};

template<class T>
struct AdapterHelper<lq::Iterators<T>>
{
    using Type = T;
    template<typename Iterator = typename T::const_iterator>
    static const Iterator& Extract(const typename lq::Iterators<T>::const_iterator& value) { return *value; }
};

template<class OutValue, class Container>
static OutValue Adapt(const typename Container::const_iterator& value) { return *value; }

struct DefaultAdapter {
    template<class Out, class Container, typename Iterator = typename Container::const_iterator>
    static std::function<Out (const Iterator& value)> For()
    {
        return [](const Iterator& value) {
            return Adapt<Out,Container>(value);
        };
    }
};

template<class T>
class Wrapper
{
public:
    using TIterator = typename T::const_iterator;
    Wrapper(const T* container)
        : m_target(container)
    {}

    Iterators<T> Select(const std::function<bool (const TIterator&)>& selector)
    {
        Iterators<T> result;
        for(auto it(m_target->begin()), e(m_target->end()); it != e; ++it) {
            if(selector(it)) {
                result.append(it);
            }
        }
        return result;
    }

    Iterators<T> Select(qint32 start, qint32 count = -1)
    {
        Iterators<T> result;
        if(count == -1) {
            count = m_target->size() - start;
        }
        auto it = m_target->cbegin() + start;
        while(count--) {
            result.append(it);
            it++;
        }
        return result;
    }

    template<class Selection, typename KeyType = typename T::key_type, typename THelper = typename lq::AdapterHelper<Selection>::Type, typename FAdapter>
    Iterators<T> SelectMap(const Selection& selection, const FAdapter& adapter)
    {
        Iterators<T> result;
        for(auto it(selection.begin()), e(selection.end()); it != e; ++it) {
            auto foundIt = m_target->find(adapter(AdapterHelper<Selection>::Extract(it)));
            if(foundIt != m_target->cend()) {
                result.append(foundIt);
            }
        }
        return result;
    }

    template<class Selection, typename KeyType = typename T::key_type, typename THelper = typename lq::AdapterHelper<Selection>::Type>
    Iterators<T> SelectMap(const Selection& selection)
    {
        return SelectMap(selection, lq::DefaultAdapter::For<KeyType, typename THelper>());
    }

    const T* operator->() const { return m_target; }

private:
    const T* m_target;
};

template<class T>
Wrapper<T> of(const T* container) { return Wrapper(container); }

template<class T, typename ... Args>
Iterators<T> select(const T& container, Args... args) { return Wrapper<T>(&container).Select(args...); }

template<class T>
struct ContainerTraits
{
    using ValueType = typename T::value_type;
};

template<class Key, class Value>
struct ContainerTraits<QMap<Key, Value>>
{
    using ValueType = Value;
};

template<class Key, class Value>
struct ContainerTraits<QHash<Key, Value>>
{
    using ValueType = Value;
};
}

class StringBuilder : public QString
{
    using Super = QString;
public:
    StringBuilder(){}
    StringBuilder(const QString& target)
        : Super(target)
    {}

    template<typename Stream, typename T, typename Container, typename FAdapter>
    static void Join(Stream& stream, const T& separator, const Container& container, const FAdapter& adapter, bool addOnFirst = false)
    {
        using Helper = lq::AdapterHelper<Container>;
        auto it(container.begin()), e(container.end());
        if(it == e) {
            return;
        }
        if(!addOnFirst) {
            adapter(Helper::Extract(it++));
        }

        for(; it != e; ++it) {
            stream << separator;
            adapter(Helper::Extract(it));
        }
        return;
    }

    template<class T>
    StringBuilder& Join(const T& separator, const QString& string)
    {
        append(separator);
        append(string);
        return *this;
    }

    template<class T, class ... Strings>
    StringBuilder& AddStrings(const T& separator, const Strings&... strings)
    {
        adapters::Combine([&](const QString& string){
            Join(separator, string);
        }, strings...);
        return *this;
    }

    template<class T, class ... Strings>
    StringBuilder& JoinStrings(const T& separator, const QString& first, const Strings&... strings)
    {
        append(first);
        adapters::Combine([&](const QString& string){
            Join(separator, string);
        }, strings...);
        return *this;
    }

    template<class T, class Container, typename FAdapter>
    StringBuilder& Join(const T& separator, const Container& container, const FAdapter& adapter, bool addOnFirst = false)
    {
        Join(*this, separator, container, [&](const auto& it){
            append(adapter(it));
        }, addOnFirst);
        return *this;
    }

    template<class T, class Container>
    StringBuilder& Join(const T& separator, const Container& container, bool addOnFirst = false)
    {
        using Helper = lq::AdapterHelper<Container>;
        return Join(separator, container, lq::DefaultAdapter::For<QString, typename Helper::Type>(), addOnFirst);
    }

    template<class T>
    StringBuilder& operator<<(const T& value) { Super::operator+=(value); return *this; }
};

template<class T>
class ContainerBuilder : public T
{
    using Super = T;
    using value_type = typename lq::ContainerTraits<T>::ValueType;
public:
    ContainerBuilder& Make(const std::function<void (ContainerBuilder&)>& handler) {
        handler(*this);
        return *this;
    }

    template<class ... Args>
    ContainerBuilder& AppendValues(Args... args)
    {
        adapters::Combine([&](const auto& it){
            this->Append(it);
        }, args...);
        return *this;
    }

    template<class Key = typename T::key_type>
    ContainerBuilder& InsertMapped(const Key& key, const value_type& value)
    {
        insert(key, value);
        return *this;
    }

    ContainerBuilder& Append(const T& another)
    {
        Super::operator+=(another);
        return *this;
    }

    template<class Container, typename FAdapter>
    ContainerBuilder& Append(const Container& data, const FAdapter& adapter)
    {
        using Helper = lq::AdapterHelper<Container>;
        for(auto it(data.begin()), e(data.end()); it != e; ++it) {
            Super::operator+=(adapter(Helper::Extract(it)));
        }
        return *this;
    }

    template<class Container>
    ContainerBuilder& Append(const Container& data)
    {
        using Helper = lq::AdapterHelper<Container>;
        return Append(data, lq::DefaultAdapter::For<typename Super::value_type, typename Helper::Type>());
    }

    ContainerBuilder& Append(const value_type& value)
    {
        Super::operator+=(value);
        return *this;
    }
};

template<class ... Args> using ParseFactoryBuilderBase = QHash<Name, std::function<void (const Args&...)>>;

template<class T>
struct ParseFactoryBuilderTextHelper
{
    using parse_type = QStringRef;
    using extractor_type = std::function<T (const QStringRef&)>;
};

template<>
struct ParseFactoryBuilderTextHelper<double>
{
    using extractor_type = std::function<double (const QStringRef&)>;
    static double Extract(const QStringRef& ref) { return ref.toDouble(); }
};

template<>
struct ParseFactoryBuilderTextHelper<qint32>
{
    using extractor_type = std::function<qint32 (const QStringRef&)>;
    static qint32 Extract(const QStringRef& ref) { return ref.toDouble(); }
};

template<>
struct ParseFactoryBuilderTextHelper<qint64>
{
    using extractor_type = std::function<qint64 (const QStringRef&)>;
    static qint64 Extract(const QStringRef& ref) { return ref.toDouble(); }
};

template<>
struct ParseFactoryBuilderTextHelper<QString>
{
    using extractor_type = std::function<QString (const QStringRef&)>;
    static QString Extract(const QStringRef& ref) { return ref.toString(); }
};

template<>
struct ParseFactoryBuilderTextHelper<bool>
{
    using extractor_type = std::function<bool (const QStringRef&)>;
    static bool Extract(const QStringRef& ref) { return ref.toDouble(); }
};

template<>
struct ParseFactoryBuilderTextHelper<Name>
{
    using extractor_type = std::function<Name (const QStringRef&)>;
    static Name Extract(const QStringRef& ref) { return Name(ref.toString()); }
};

template<typename T>
struct ParseFactoryBuilderTextHelper<std::optional<T>>
{
    using extractor_type = std::function<std::optional<T> (const QStringRef&)>;
    static std::optional<T> Extract(const QStringRef& ref) { return TextConverter<std::optional<T>>::FromText(ref); }
};

template<template<class T> typename Helper, class Context>
class ParseFactoryBuilder : public ParseFactoryBuilderBase<const typename Helper<void>::parse_type&, Context&>
{
    using Super = ParseFactoryBuilderBase<const typename Helper<void>::parse_type&, Context&>;
    using FFunctor = typename Super::const_iterator::value_type;
    template<class T> using FTargetExtractor = std::function<T* (Context&)>;
    template<class T, class T2> using FPropertyExtractor = std::function<T& (T2&)>;
public:
    using Super::Super;

    template<class T2>
    class ParseFactoryBuilderObject
    {
    public:
        ParseFactoryBuilderObject(ParseFactoryBuilder* builder, const FTargetExtractor<T2>& targetExtractor)
            : m_builder(builder)
            , m_targetExtractor(targetExtractor)
        {}

        template<typename Enum>
        const ParseFactoryBuilderObject& RegisterEnum(const Name& key,
                                      const FPropertyExtractor<Enum,T2>& targetPropertyExtractor,
                                      const typename Helper<qint64>::extractor_type& extractor = &Helper<qint64>::Extract) const
        {
            m_builder->RegisterEnum<Enum, T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<typename Enum, typename value_type = typename LocalPropertySequentialEnum<Enum>::value_type>
        const ParseFactoryBuilderObject& RegisterPropertyEnum(const Name& key,
                                      const FPropertyExtractor<LocalPropertySequentialEnum<Enum>,T2>& targetPropertyExtractor,
                                      const typename Helper<value_type>::extractor_type& extractor = &Helper<value_type>::Extract) const
        {
            m_builder->RegisterPropertyEnum<Enum, T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        const ParseFactoryBuilderObject& Insert(const Name& key, const std::function<void (const QStringRef&, T2&)>& handler) const
        {
            auto extractor = m_targetExtractor;
            m_builder->Insert(key, [extractor, handler](const QStringRef& ref, Context& context){
                auto* target = extractor(context);
                if(target == nullptr) {
                    return;
                }
                handler(ref, *target);
            });
            return *this;
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key, const class MeasurementUnit& unit,
                                      const FPropertyExtractor<double,T2>& targetPropertyExtractor,
                                      const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract) const
        {
            m_builder->RegisterMeasurementField<T2>(key, unit, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key, const class MeasurementUnit& unit,
                                      const FPropertyExtractor<LocalPropertyDouble,T2>& targetPropertyExtractor,
                                      const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract) const
        {
            m_builder->RegisterMeasurementProperty<T2>(key, unit, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                      const FPropertyExtractor<std::optional<double>,T2>& targetPropertyExtractor,
                                      const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract) const
        {
            m_builder->RegisterOptional<double, T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }


        template<typename Property, typename T = typename Property::value_type>
        const ParseFactoryBuilderObject& RegisterProperty(const Name& key,
                                      const FPropertyExtractor<Property,T2>& targetPropertyExtractor,
                                      const typename Helper<T>::extractor_type& extractor = &Helper<T>::Extract) const
        {
            m_builder->RegisterProperty<Property, T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<class Enum, typename value_type = typename LocalPropertySequentialEnum<Enum>::value_type>
        const ParseFactoryBuilderObject& RegisterEnum(const Name& key,
                                      const FPropertyExtractor<LocalPropertySequentialEnum<Enum>,T2>& targetPropertyExtractor,
                                      const typename Helper<value_type>::extractor_type& extractor = &Helper<value_type>::Extract) const
        {
            return RegisterPropertyEnum<Enum>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterBool(const Name& key,
                                      const FPropertyExtractor<bool,T2>& targetPropertyExtractor,
                                      const typename Helper<bool>::extractor_type& extractor = &Helper<bool>::Extract) const
        {
            return RegisterField<bool>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterBool(const Name& key,
                                      const FPropertyExtractor<LocalPropertyBool,T2>& targetPropertyExtractor,
                                      const typename Helper<bool>::extractor_type& extractor = &Helper<bool>::Extract) const
        {
            return RegisterProperty<LocalPropertyBool>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterId(const Name& key,
                                      const FPropertyExtractor<Name,T2>& targetPropertyExtractor,
                                      const typename Helper<Name>::extractor_type& extractor = &Helper<Name>::Extract) const
        {
            return RegisterField<Name>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterId(const Name& key,
                                      const FPropertyExtractor<LocalProperty<Name>,T2>& targetPropertyExtractor,
                                      const typename Helper<Name>::extractor_type& extractor = &Helper<Name>::Extract) const
        {
            return RegisterProperty<LocalProperty<Name>>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterIdPointer(const Name& key,
                                      const FPropertyExtractor<Name*,T2>& targetPropertyExtractor,
                                      const typename Helper<Name>::extractor_type& extractor = &Helper<Name>::Extract) const
        {
            return RegisterFieldPointer<Name>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key, const MeasurementUnit& unit,
                                      const FPropertyExtractor<std::optional<double>,T2>& targetPropertyExtractor,
                                      const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract) const
        {
            m_builder->RegisterMeasurementOptional<T2>(key, unit, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key, const MeasurementUnit& unit,
                                      const FPropertyExtractor<LocalPropertyDoubleOptional,T2>& targetPropertyExtractor,
                                      const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract) const
        {
            m_builder->RegisterMeasurementPropertyOptional<T2>(key, unit, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<typename T>
        const ParseFactoryBuilderObject& RegisterField(const Name& key,
                                      const FPropertyExtractor<T,T2>& targetPropertyExtractor,
                                      const typename Helper<T>::extractor_type& extractor = &Helper<T>::Extract) const
        {
            m_builder->RegisterField<T, T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<typename T>
        const ParseFactoryBuilderObject& RegisterFieldPointer(const Name& key,
                                      const FPropertyExtractor<T*,T2>& targetPropertyExtractor,
                                      const typename Helper<T>::extractor_type& extractor = &Helper<T>::Extract) const
        {
            m_builder->RegisterField<T, T2>(key, [targetPropertyExtractor](T2& context) -> T& { return *targetPropertyExtractor(context); }, m_targetExtractor, extractor);
            return *this;
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                                        const FPropertyExtractor<LocalPropertyInt,T2>& targetPropertyExtractor,
                                                        const typename Helper<qint32>::extractor_type& extractor = &Helper<qint32>::Extract) const
        {
            return RegisterProperty<LocalPropertyInt>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                                        const FPropertyExtractor<std::optional<qint32>,T2>& targetPropertyExtractor,
                                                        const typename Helper<std::optional<qint32>>::extractor_type& extractor = &Helper<std::optional<qint32>>::Extract) const
        {
            return RegisterOptional<qint32>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                                        const FPropertyExtractor<qint32,T2>& targetPropertyExtractor,
                                                        const typename Helper<qint32>::extractor_type& extractor = &Helper<qint32>::Extract) const
        {
            return RegisterField<qint32>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                                        const FPropertyExtractor<LocalPropertyIntOptional,T2>& targetPropertyExtractor,
                                                        const typename Helper<std::optional<qint32>>::extractor_type& extractor = &Helper<std::optional<qint32>>::Extract) const
        {
            return RegisterPropertyOptional<LocalPropertyIntOptional>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                                        const FPropertyExtractor<LocalPropertyDoubleOptional,T2>& targetPropertyExtractor,
                                                        const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract) const
        {
            return RegisterPropertyOptional<LocalPropertyDoubleOptional>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                                        const FPropertyExtractor<LocalPropertyDouble,T2>& targetPropertyExtractor,
                                                        const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract) const
        {
            return RegisterProperty<LocalPropertyDouble>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterDouble(const Name& key,
                                                        const FPropertyExtractor<double,T2>& targetPropertyExtractor,
                                                        const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract) const
        {
            m_builder->RegisterField<double, T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        const ParseFactoryBuilderObject& RegisterString(const Name& key,
                                                        const FPropertyExtractor<LocalPropertyString,T2>& targetPropertyExtractor,
                                                        const typename Helper<QString>::extractor_type& extractor = &Helper<QString>::Extract) const
        {
            return RegisterProperty<LocalPropertyString>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterString(const Name& key,
                                                        const FPropertyExtractor<LocalPropertyStringOptional,T2>& targetPropertyExtractor,
                                                        const typename Helper<std::optional<QString>>::extractor_type& extractor = &Helper<std::optional<QString>>::Extract) const
        {
            return RegisterPropertyOptional<LocalPropertyStringOptional>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterString(const Name& key,
                                                        const FPropertyExtractor<std::optional<QString>,T2>& targetPropertyExtractor,
                                                        const typename Helper<std::optional<QString>>::extractor_type& extractor = &Helper<std::optional<QString>>::Extract) const
        {
            return RegisterOptional<QString>(key, targetPropertyExtractor, extractor);
        }

        const ParseFactoryBuilderObject& RegisterString(const Name& key,
                                                        const FPropertyExtractor<QString,T2>& targetPropertyExtractor,
                                                        const typename Helper<QString>::extractor_type& extractor = &Helper<QString>::Extract) const
        {
            return RegisterField<QString>(key, targetPropertyExtractor, extractor);
        }

        template<typename T, typename value_type = typename T::value_type>
        const ParseFactoryBuilderObject& RegisterPropertyOptional(const Name& key,
                                      const FPropertyExtractor<T,T2>& targetPropertyExtractor,
                                      const typename Helper<std::optional<value_type>>::extractor_type& extractor = &Helper<std::optional<value_type>>::Extract) const
        {
            m_builder->RegisterPropertyOptional<T,T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<typename T>
        const ParseFactoryBuilderObject& RegisterOptional(const Name& key,
                                      const FPropertyExtractor<std::optional<T>,T2>& targetPropertyExtractor,
                                      const typename Helper<std::optional<T>>::extractor_type& extractor = &Helper<std::optional<T>>::Extract) const
        {
            m_builder->RegisterOptional<T,T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<class NewContext>
        ParseFactoryBuilderObject MakeObject(const FTargetExtractor<NewContext>& extractor) const { return m_builder->MakeObject<NewContext>(extractor); }

    private:
        ParseFactoryBuilder* m_builder;
        FTargetExtractor<T2> m_targetExtractor;
    };


    template<typename T, typename T2, typename value_type = typename T::value_type>
    ParseFactoryBuilder& RegisterPropertyOptional(const Name& key,
                                  const FPropertyExtractor<T,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<std::optional<value_type>>::extractor_type& extractor = &Helper<std::optional<value_type>>::Extract)
    {
        return Insert(key, [targetPropertyExtractor, targetExtractor, extractor](const typename Helper<void>::parse_type& toParse, Context& context){
            auto* target = targetExtractor(context);
            if(target == nullptr) {
                return;
            }
            targetPropertyExtractor(*target).SetSilentWithValidators(extractor(toParse));
        });
    }

    template<typename T, typename T2>
    ParseFactoryBuilder& RegisterOptional(const Name& key,
                                  const FPropertyExtractor<std::optional<T>,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<std::optional<T>>::extractor_type& extractor = &Helper<std::optional<T>>::Extract)
    {
        return Insert(key, [targetPropertyExtractor, targetExtractor, extractor](const typename Helper<void>::parse_type& toParse, Context& context){
            auto* target = targetExtractor(context);
            if(target == nullptr) {
                return;
            }
            targetPropertyExtractor(*target) = extractor(toParse);
        });
    }

    template<typename Enum, typename T2>
    ParseFactoryBuilder& RegisterEnum(const Name& key,
                                  const FPropertyExtractor<Enum,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<qint64>::extractor_type& extractor = &Helper<qint64>::Extract)
    {
        return Insert(key, [targetPropertyExtractor, targetExtractor, extractor](const typename Helper<void>::parse_type& toParse, Context& context){
            auto* target = targetExtractor(context);
            if(target == nullptr) {
                return;
            }
            targetPropertyExtractor(*target) = EnumHelper<void>::Validate<Enum>(extractor(toParse));
        });
    }

    template<typename Enum, typename T2, typename value_type = typename LocalPropertySequentialEnum<Enum>::value_type>
    ParseFactoryBuilder& RegisterPropertyEnum(const Name& key,
                                  const FPropertyExtractor<LocalPropertySequentialEnum<Enum>,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<value_type>::extractor_type& extractor = &Helper<value_type>::Extract)
    {
        return RegisterProperty<LocalPropertySequentialEnum<Enum>, T2>(key, targetPropertyExtractor, targetExtractor, extractor);
    }

    template<typename Property, typename T2, typename T = typename Property::value_type>
    ParseFactoryBuilder& RegisterProperty(const Name& key,
                                  const FPropertyExtractor<Property,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<T>::extractor_type& extractor = &Helper<T>::Extract)
    {
        return Insert(key, [extractor, targetPropertyExtractor, targetExtractor](const typename Helper<void>::parse_type& toParse, Context& context){
            auto* target = targetExtractor(context);
            if(target == nullptr) {
                return;
            }
            targetPropertyExtractor(*target).SetSilentWithValidators(extractor(toParse));
        });
    }

    template<class T2>
    ParseFactoryBuilder& RegisterId(const Name& key,
                                  const FPropertyExtractor<Name,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor = &GlobalSelfGetterPointer<T2>,
                                  const typename Helper<Name>::extractor_type& extractor = &Helper<Name>::Extract)
    {
        return RegisterField<Name>(key, targetPropertyExtractor, targetExtractor, extractor);
    }

    template<class T2>
    ParseFactoryBuilder& RegisterId(const Name& key,
                                  const FPropertyExtractor<LocalProperty<Name>,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor = &GlobalSelfGetterPointer<T2>,
                                  const typename Helper<Name>::extractor_type& extractor = &Helper<Name>::Extract)
    {
        return RegisterProperty<LocalProperty<Name>>(key, targetPropertyExtractor, targetExtractor, extractor);
    }

    template<typename T, class T2>
    ParseFactoryBuilder& RegisterField(const Name& key,
                                  const FPropertyExtractor<T,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor = &GlobalSelfGetterPointer<T2>,
                                  const typename Helper<T>::extractor_type& extractor = &Helper<T>::Extract)
    {
        return Insert(key, [extractor, targetExtractor, targetPropertyExtractor](const typename Helper<void>::parse_type& in, Context& context){
            auto* target = targetExtractor(context);
            if(target == nullptr) {
                return;
            }
            targetPropertyExtractor(*target) = extractor(in);
        });
    }

    ParseFactoryBuilder& Insert(const Name& key, const FFunctor& extractor)
    {
        Q_ASSERT(!Super::contains(key));
        Super::insert(key, extractor);
        return *this;
    }

    template<class T2>
    ParseFactoryBuilder& RegisterMeasurementOptional(const Name& key, const MeasurementUnit& unit,
                                  const FPropertyExtractor<std::optional<double>,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract)
    {
        return RegisterOptional<double, T2>(key, targetPropertyExtractor, targetExtractor, [extractor, &unit](const typename Helper<void>::parse_type& ref) -> std::optional<double> {
            auto result = extractor(ref);
            if(result.has_value()) {
                return unit.FromUnitToBase(result.value());
            }
            return result;
        });
    }

    template<class T2>
    ParseFactoryBuilder& RegisterMeasurementPropertyOptional(const Name& key, const MeasurementUnit& unit,
                                  const FPropertyExtractor<LocalPropertyDoubleOptional,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract)
    {
        return RegisterPropertyOptional<LocalPropertyDoubleOptional, T2>(key, targetPropertyExtractor, targetExtractor, [extractor, &unit](const typename Helper<void>::parse_type& ref) -> std::optional<double> {
            auto result = extractor(ref);
            if(result.has_value()) {
                return unit.FromUnitToBase(result.value());
            }
            return result;
        });
    }

    template<class T2>
    ParseFactoryBuilder& RegisterMeasurementProperty(const Name& key, const MeasurementUnit& unit,
                                  const FPropertyExtractor<LocalPropertyDouble,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract)
    {
        return RegisterProperty<LocalPropertyDouble, T2>(key, targetPropertyExtractor, targetExtractor, [extractor, &unit](const typename Helper<void>::parse_type& ref) -> double {
            return unit.FromUnitToBase(extractor(ref));
        });
    }

    template<class T2>
    ParseFactoryBuilder& RegisterMeasurementField(const Name& key, const MeasurementUnit& unit,
                                  const FPropertyExtractor<double,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract)
    {
        return Insert(key, [extractor, targetExtractor, targetPropertyExtractor, &unit](const typename Helper<void>::parse_type& in, Context& context){
            auto* target = targetExtractor(context);
            if(target == nullptr) {
                return;
            }
            targetPropertyExtractor(*target) = unit.FromUnitToBase(extractor(in));
        });
    }

    ParseFactoryBuilder& Build(const std::function<void (ParseFactoryBuilder&)>& builder)
    {
        builder(*this);
        return *this;
    }

    template<typename T>
    ParseFactoryBuilderObject<T> MakeObject(const FTargetExtractor<T>& targetExtractor)
    {
        return ParseFactoryBuilderObject<T>(this, targetExtractor);
    }

    template<typename T>
    ParseFactoryBuilderObject<T> MakeObject(const FTargetExtractor<T*>& targetExtractor)
    {
        return MakeObject<T>(this, [targetExtractor](Context& context) -> T& { return *targetExtractor(context); });
    }

    ParseFactoryBuilderObject<Context> MakeObject()
    {
        return ParseFactoryBuilderObject<Context>(this, &GlobalSelfGetterPointer<Context>);
    }

    const ParseFactoryBuilder& ParseXmlAttributes(QXmlStreamReader& xml, const Context& context) const
    {
        auto attributes = xml.attributes();
        for (const auto& attribut : attributes) {
            auto attrName = Name(attribut.name().toString().toLower());
            const auto& attrValue = attribut.value();

            auto foundIt = find(attrName);
            if(foundIt != cend()) {
                foundIt.value()(attrValue, const_cast<Context&>(context));
            }
        }
        return *this;
    }
};

template<class ... Context>
class FactoryBuilder : public ParseFactoryBuilderBase<Context...>
{
    using Super = ParseFactoryBuilderBase<Context...>;
public:

    FactoryBuilder<Context...>& Insert(const Name& name, const std::function<void (Context...)>& handler)
    {
        Super::insert(name, handler);
        return *this;
    }

    bool Process(const Name& name, Context... context) const
    {
        auto foundIt = Super::find(name);
        if(foundIt != Super::cend()) {
            foundIt.value()(context...);
            return true;
        }
        return false;
    }
};


#endif // BUILDERS_H
