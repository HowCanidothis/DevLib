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
    static bool Extract(const QStringRef& ref) { return ref.toInt(); }
};

template<>
struct ParseFactoryBuilderTextHelper<Name>
{
    using extractor_type = std::function<Name (const QStringRef&)>;
    static Name Extract(const QStringRef& ref) { return Name(ref.toString()); }
};

template<template<class T> typename Helper, class Context>
class ParseFactoryBuilder : public ParseFactoryBuilderBase<const typename Helper<void>::parse_type&, Context&>
{
    using Super = ParseFactoryBuilderBase<const typename Helper<void>::parse_type&, Context&>;
    using FFunctor = typename Super::const_iterator::value_type;
    template<class T> using FTargetExtractor = std::function<T& (Context&)>;
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

        const ParseFactoryBuilderObject& Insert(const Name& key, const std::function<void (const QStringRef&, T2&)>& handler) const
        {
            auto extractor = m_targetExtractor;
            m_builder->Insert(key, [extractor, handler](const QStringRef& ref, Context& context){ handler(ref, extractor(context)); });
            return *this;
        }

        const ParseFactoryBuilderObject& RegisterMeasurementField(const Name& key, const class MeasurementUnit& unit,
                                      const FPropertyExtractor<double,T2>& targetPropertyExtractor,
                                      const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract) const
        {
            m_builder->RegisterMeasurementField(key, unit, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<typename Property, typename T = typename Property::value_type>
        const ParseFactoryBuilderObject& RegisterProperty(const Name& key,
                                      const FPropertyExtractor<Property,T2>& targetPropertyExtractor,
                                      const typename std::function<T (const typename Helper<void>::parse_type&)>& extractor) const
        {
            auto targetExtractor = m_targetExtractor;
            m_builder->Insert(key, [extractor, targetPropertyExtractor, targetExtractor](const typename Helper<void>::parse_type& toParse, Context& context){
                targetPropertyExtractor(targetExtractor(context)).SetSilentWithValidators(extractor(toParse));
            });
            return *this;
        }

        template<typename Property, typename T = typename Property::value_type>
        const ParseFactoryBuilderObject& RegisterProperty(const Name& key,
                                      const FPropertyExtractor<Property,T2>& targetPropertyExtractor) const
        {
            RegisterProperty<Property, T>(key, targetPropertyExtractor, &Helper<T>::Extract);
            return *this;
        }

        template<typename Enum>
        const ParseFactoryBuilderObject& RegisterPropertyEnum(const Name& key,
                                      const FPropertyExtractor<LocalPropertySequentialEnum<Enum>,T2>& targetPropertyExtractor) const
        {
            RegisterPropertyEnum<Enum>(key, targetPropertyExtractor, &Helper<typename LocalPropertySequentialEnum<Enum>::value_type>::Extract);
            return *this;
        }

        template<typename Enum>
        const ParseFactoryBuilderObject& RegisterPropertyEnum(const Name& key,
                                      const FPropertyExtractor<LocalPropertySequentialEnum<Enum>,T2>& targetPropertyExtractor,
                                      const typename std::function<typename LocalPropertySequentialEnum<Enum>::value_type (const typename Helper<void>::parse_type&)>& extractor) const
        {
            RegisterProperty<LocalPropertySequentialEnum<Enum>, typename LocalPropertySequentialEnum<Enum>::value_type>(key, targetPropertyExtractor, extractor);
            return *this;
        }

        template<typename T>
        const ParseFactoryBuilderObject& RegisterField(const Name& key,
                                      const FPropertyExtractor<T,T2>& targetPropertyExtractor,
                                      const typename std::function<T (const typename Helper<void>::parse_type&)>& extractor) const
        {
            m_builder->RegisterField<T, T2>(key, targetPropertyExtractor, m_targetExtractor, extractor);
            return *this;
        }

        template<typename T>
        const ParseFactoryBuilderObject& RegisterField(const Name& key,
                                      const FPropertyExtractor<T,T2>& targetPropertyExtractor) const
        {
            RegisterField<T>(key, targetPropertyExtractor, &Helper<T>::Extract);
            return *this;
        }

    private:
        ParseFactoryBuilder* m_builder;
        FTargetExtractor<T2> m_targetExtractor;
    };

    template<typename T, class T2>
    ParseFactoryBuilder& RegisterField(const Name& key,
                                  const FPropertyExtractor<T,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor = &GlobalSelfGetter<T2>,
                                  const typename std::function<T (const typename Helper<void>::parse_type&)>& extractor = &Helper<T>::Extract)
    {
        Super::insert(key, [extractor, targetExtractor, targetPropertyExtractor](const typename Helper<void>::parse_type& in, Context& context){
            targetPropertyExtractor(targetExtractor(context)) = extractor(in);
        });
        return *this;
    }

    ParseFactoryBuilder& Insert(const Name& key, const FFunctor& extractor)
    {
        Super::insert(key, extractor);
        return *this;
    }

    template<class T2>
    ParseFactoryBuilder& RegisterMeasurementField(const Name& key, const MeasurementUnit& unit,
                                  const FPropertyExtractor<double,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor,
                                  const typename Helper<double>::extractor_type& extractor);

    template<class T2>
    ParseFactoryBuilder& RegisterMeasurementField(const Name& key, const MeasurementUnit& unit,
                                  const FPropertyExtractor<double,T2>& targetPropertyExtractor,
                                  const FTargetExtractor<T2>& targetExtractor)
    {
        return RegisterMeasurementField<T2>(key, unit, targetPropertyExtractor, targetExtractor, &Helper<double>::Extract);
    }

    template<class T2>
    ParseFactoryBuilder& RegisterMeasurementField(const Name& key, const MeasurementUnit& unit,
                                  const FPropertyExtractor<double,T2>& targetPropertyExtractor)
    {
        return RegisterMeasurementField<T2>(key, unit, targetPropertyExtractor, &GlobalSelfGetter<T2>, &Helper<double>::Extract);
    }

    template<typename T>
    ParseFactoryBuilder& MakeObject(const FTargetExtractor<T>& targetExtractor,
                               const std::function<void (const ParseFactoryBuilderObject<T>& builder)>& registerHandler)
    {
        registerHandler(ParseFactoryBuilderObject<T>(this, targetExtractor));
        return *this;
    }

    template<typename T>
    ParseFactoryBuilder& MakeObject(const std::function<void (const ParseFactoryBuilderObject<T>& builder)>& registerHandler)
    {
        registerHandler(ParseFactoryBuilderObject<T>(this, &GlobalSelfGetter<T>));
        return *this;
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
