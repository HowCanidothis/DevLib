#ifndef ERRORS_H
#define ERRORS_H

template<class... Input>
struct TErrorParams
{
    using FValidator = std::function<bool (const Input&...)>;
    TErrorParams(const Name& errorKey, const FTranslationHandler& errorText, const std::function<bool (const Input&...)>& validator)
        : ErrorKey(errorKey)
        , ErrorText(errorText)
        , Validator(validator)
    {}

    ~TErrorParams()
    {}

    Name ErrorKey;
    FTranslationHandler ErrorText;
    FValidator Validator;
};

template<typename Flags, class... Input>
class TErrorParamsContainer
{
public:
    using Params = TErrorParams<Input...>;
    using ValidatorValue = std::pair<const Flags, Params>;
    using PutValue = typename std::map<Flags, Params>::const_iterator;
    using MappedResult = QMap<Flags, QVector<PutValue>>;

    class Output
    {
    public:
        using FValidator = std::function<void (Output&, const Input&...)>;

        Output()
            : m_resultFlags((Flags)0)
            , m_resultFlagsCollector([](Flags){})
            , m_errorCollector([](const PutValue&){})
            , m_mappedResultCollector([](const PutValue&){})
        {}

        Output& EnableMappedResult()
        {
            m_mappedResultCollector = [this](const PutValue& value){
                m_mappedResult[value->first].append(value);
            };
            return *this;
        }
        Output& EnableResultFlags() { m_resultFlagsCollector = [this](Flags flags){ m_resultFlags.AddFlags(flags); }; return *this; }
        Output& EnableResult() { m_errorCollector = [this](const PutValue& error){ m_error.append(error); }; return *this; }

        Flags GetResultFlags() const { return m_resultFlags; }
        const QVector<PutValue>& GetResult() const { return m_error; }
        const MappedResult& GetMappedResult() const { return m_mappedResult; }

    private:
        void put(const PutValue& iterator)
        {
            m_resultFlagsCollector(iterator->first);
            m_errorCollector(iterator);
            m_mappedResultCollector(iterator);
        }

    private:
        friend class TErrorParamsContainer;
        Flags m_resultFlags;
        QVector<PutValue> m_error;
        MappedResult m_mappedResult;

        std::function<void (Flags)> m_resultFlagsCollector;
        std::function<void (const PutValue&)> m_errorCollector;
        std::function<void (const PutValue&)> m_mappedResultCollector;
    };

    using FValidator = typename Output::FValidator;

    TErrorParamsContainer(std::initializer_list<ValidatorValue> list)
        : m_validators(list)
    {

    }

    void FillValidator(Flags flags, FValidator& result) const
    {
        for(auto it(m_validators.cbegin()), e(m_validators.cend()); it != e; ++it) {
            const auto& key = it->first;
            const auto* value = &it->second;
            if(flags.TestFlagsAll(key)) {
                auto oldResult = result;
                auto validator = value->Validator;
                const auto& cvalue = value;
                auto ckey = key;
                if(oldResult == nullptr) {
                    result = [it, validator](Output& result, const Input&... context){
                        if(!validator(context...)) {
                            result.put(it);
                        }
                    };
                    continue;
                }
                result = [it, oldResult, validator](Output& result, const Input&... context){
                    oldResult(result, context...);
                    if(!validator(context...)) {
                        result.put(it);
                    }
                };
            }
        }
    }

    const Params& Find(Flags flags) const
    {
        auto foundIt = m_validators.find(flags);
        if(foundIt != m_validators.end()) {
            return foundIt->second;
        }
        return Default<Params>::Value;
    }

    static ValidatorValue CreateValue(Flags flags, const Name& errorKey, const FTranslationHandler& errorText, const typename Params::FValidator& validator)
    {
        return std::make_pair(flags, Params(errorKey, errorText, validator));
    }

private:
    std::map<Flags, Params> m_validators;
};

#endif // ERRORS_H
