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

    class Output
    {
    public:
        using FValidator = std::function<void (Output&, const Input&...)>;

        Output()
            : m_resultFlags((Flags)0)
            , m_resultFlagsCollector([](Flags){})
            , m_resultFlagsContainerCollector([](Flags){})
            , m_errorCollector([](const Params*){})
        {}

        Output& EnableResultFlags() { m_resultFlagsCollector = [this](Flags flags){ m_resultFlags.AddFlags(flags); }; return *this; }
        Output& EnableResultFlagsContainer() { m_resultFlagsContainerCollector = [this](Flags flags){ m_resultFlagsContainer.append(flags); }; return *this; }
        Output& EnableResult() { m_errorCollector = [this](const Params* error){ m_error.append(error); }; return *this; }

        Flags GetResultFlags() const { return m_resultFlags; }
        const QVector<Flags>& GetResultFlagsContainer() const { return m_resultFlagsContainer; }
        const QVector<const Params*>& GetErrors() const { return m_error; }

    private:
        void put(Flags flags, const Params* value)
        {
            m_resultFlagsCollector(flags);
            m_resultFlagsContainerCollector(flags);
            m_errorCollector(value);
        }

    private:
        friend class TErrorParamsContainer;
        Flags m_resultFlags;
        QVector<Flags> m_resultFlagsContainer;
        QVector<const Params*> m_error;

        std::function<void (Flags)> m_resultFlagsCollector;
        std::function<void (Flags)> m_resultFlagsContainerCollector;
        std::function<void (const Params*)> m_errorCollector;
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
                    result = [value, validator, ckey](Output& result, const Input&... context){
                        if(!validator(context...)) {
                            result.put(ckey, value);
                        }
                    };
                    continue;
                }
                result = [value, oldResult, validator, ckey](Output& result, const Input&... context){
                    oldResult(result, context...);
                    if(!validator(context...)) {
                        result.put(ckey, value);
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
