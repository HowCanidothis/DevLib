#ifndef STATEPROPERTY_H
#define STATEPROPERTY_H

#include "localproperty.h"
#include "SharedModule/exception.h"

template<class T> class StateCalculator;

class StateProperty : public LocalPropertyBool
{
    using Super = LocalPropertyBool;
public:
    using Super::Super;

    void SetState(bool state);

    DispatcherConnections ConnectFromStateProperty(const char* location, const StateProperty& property);
    template<typename ... Properties>
    DispatcherConnections ConnectFromStateProperty(const char* location, const LocalPropertyBool& property, const Properties&... props)
    {
        DispatcherConnections result;
        auto update = [this, &property, &props...] {
            bool valid = true;
            for(bool prop : {property.Native(), props.Native()...}) {
                if(!prop) {
                    valid = false;
                    break;
                }
            }
            SetState(valid);
        };
        adapters::Combine([&](const auto& property){
            result += property.OnChanged.Connect(location, update);
        }, property, props...);
        update();
        return result;
    }

    template<typename ... Args>
    static DispatcherConnection OnFirstInvokePerformWhenEveryIsValid(const char* location, const FAction& handler, const Args&... stateProperties)
    {
        auto commutator = ::make_shared<WithDispatcherConnectionsSafe<LocalPropertyBoolCommutator>>(LocalPropertyBoolCommutator::And);
        commutator->ConnectFrom(location, stateProperties...).MakeSafe(commutator->Connections);

        return commutator->AsProperty().OnChanged.OnFirstInvoke([location, handler, commutator]{
            handler();
            ThreadsBase::DoMain(location, [commutator]{}); // Safe deletion
        });
    }

    static DispatcherConnection OnFirstInvokePerformWhenEveryIsValid(const char* location, const FAction& handler, const QVector<const StateProperty*>& stateProperties);
    static bool Wait(const char* cdl, const QVector<const StateProperty*>& stateProperties, qint32 msecs = -1);
};

class StatePropertyBoolCommutator : public LocalPropertyBoolCommutatorBase<DispatchersCommutatorWithDirect>
{
    using Super = LocalPropertyBoolCommutatorBase<DispatchersCommutatorWithDirect>;
public:
    StatePropertyBoolCommutator(bool defaultState = true);

    void Update();

#ifdef QT_DEBUG
    const Dispatcher& OnDirectChanged;
#endif
};

class LocalPropertyErrorsModel;
class StateParametersGroup;

#define SPCO StateParameters::ChainOptions
#define SPCCO StateParameters::ChainConnectionOptions

class StateParameters
{
public:
    StateParameters(bool valid = true);

    class ChainOptions
    {
    public:
        ChainOptions()
            : Includable(false)
            , Included(false)
            , InjectErrors(false)
        {}

        ~ChainOptions()
        {
        }

        bool Includable;
        bool Included;
        bool InjectErrors;


        ChainOptions& SetIncludable() { Includable = true; return *this; }
        ChainOptions& SetNotIncluded() { Included = false; return *this; }
        ChainOptions& AddInjectedErrors() { InjectErrors = true; return *this; }
        Q_DISABLE_COPY(ChainOptions);
    };

    void SetId(const QString& id)
    {
#ifdef QT_DEBUG
        Id = id;
#endif
    }

#ifdef QT_DEBUG
    QString Id;
#endif

    struct ChainConnectionOptions
    {
        QVector<const LocalPropertyErrorsModel*> Errors;
        QVector<const LocalPropertyErrorsModel*> Processes;
        QVector<SmartPointer*> Capturers;
        Name ProcessId;

        ChainConnectionOptions& SetProcessId(const Name& id) { ProcessId = id; return *this; }

        template<typename ... Args>
        ChainConnectionOptions& AddCapturers(SmartPointer* capture, const Args&... captures)
        {
             adapters::Combine([&](auto* capturer) {
                Capturers.append(capturer);
            }, capture, captures...);
            return *this;
        }

        template<typename ... Args>
        ChainConnectionOptions& AddErrors(const LocalPropertyErrorsModel& eModel, const Args&... args)
        {
            adapters::Combine([&](const auto& errorModel) {
                Errors.append(&errorModel);
            }, eModel, args...);
            return *this;
        }

        template<typename ... Args>
        ChainConnectionOptions& AddProcesses(const LocalPropertyErrorsModel& eModel, const Args&... args)
        {
            adapters::Combine([&](const auto& errorModel) {
                Processes.append(&errorModel);
            }, eModel, args...);
            return *this;
        }
    };

    // For debug proposes only. Use it for params which does not have errors, captures but still is used in chain
    void SetEmptyChained()
    {
#ifdef QT_DEBUG
        m_hasEmptyChainData = true;
#endif
    }

    // Input parameter with errors support
    void SetChained(const ChainOptions& params = ChainOptions());
    void DisconnectChain();
    template<class T>
    void ConnectChainWithCalculator(const char* cdl, const ChainConnectionOptions& options, StateCalculator<T>& calculator)
    {
        calculator.OnCalculated += { this, [this](const auto& container){
            GetChainData()->EditErrors().Remove(nullptr);
        }};
        calculator.OnExceptionCaught += { this, [this](const Exception& ex) {
            GetChainData()->EditErrors().Add(ex.GetError(), nullptr);
        }};
        ConnectChain(cdl, const_cast<ChainConnectionOptions&>(options).AddCapturers(calculator.GetCapturer()));
    }
    void ConnectChain(const char* cdl, const ChainConnectionOptions& params);
    void ConnectChain(const char* cdl, const std::function<void (LocalPropertyErrorsModel&)>& errorInitializer, const ChainConnectionOptions& params = ChainConnectionOptions())
    {
        errorInitializer(GetInjectedErrors());
        ConnectChain(cdl, params);
    }

    template<class T, typename ... Args>
    void ConnectChain(const char* cdl, const ChainConnectionOptions& chainParams, const T& param1, const Args&... params)
    {
        auto& cp = const_cast<ChainConnectionOptions&>(chainParams);
        adapters::Combine([&](const auto& param){
            extract(param, cp.Errors, cp.Processes, cp.Capturers);
        }, param1, params...);

        ConnectChain(cdl, cp);
    }

    void ConnectChainAsProxyOf(const char* cdl, const SP<StateParameters>& another, const ChainConnectionOptions& options = ChainConnectionOptions());

    void Initialize();
    bool IsInitialized() { return m_initializer == nullptr; }

    void Lock();
    void Unlock();
    void Reset();

    LocalPropertyErrorsModel& GetInjectedErrors();
    class StateParametersChainData* GetChainData() { return m_chainData.get(); }
    bool IsIncludedImmutable() const;
    LocalPropertyBool& GetIncluded() const;
    const LocalPropertyErrorsModel& GetErrors() const;
    const LocalPropertyErrorsModel& GetProcesses() const;
    SmartPointer* GetCapturer() const;

    StatePropertyBoolCommutator IsValid;
    Dispatcher OnChanged; // StateParameters are primary used in Calculators, then calculators must be reset when any property changed immediatly
    LocalPropertyBool IsLocked;
    DispatcherConnectionsSafe Connections;

protected:
    virtual void onInitialized() {}

private:
    void setIncludable();
    void extract(const SP<StateParameters>& params, QVector<const LocalPropertyErrorsModel*>& errors, QVector<const LocalPropertyErrorsModel*>& processes, QVector<SmartPointer*>& captures);
    void extract(const StateParametersGroup& wrapper, QVector<const LocalPropertyErrorsModel*>& errors, QVector<const LocalPropertyErrorsModel*>& processes, QVector<SmartPointer*>& captures);

private:
    QVector<class IStateParameterBase*> m_parameters;

private:
    friend class IStateParameterBase;
    template<class T> friend class StateCalculator;
    std::atomic_int m_counter;
    StateProperty m_isValid;
    FAction m_initializer;
    SP<StateParametersChainData> m_chainData;
    mutable ScopedPointer<LocalPropertyErrorsModel> m_injectedErrors;
#ifdef QT_DEBUG
    bool m_hasEmptyChainData = false;
#endif
};

template<class T>
SP<T> make_chained(const SP<T>& target)
{
    target->SetEmptyChained();
    return std::move(target);
}

template<class T>
SP<T> make_chained(const SP<T>& target, const StateParameters::ChainOptions& chainOptions)
{
    target->SetChained(chainOptions);
    return std::move(target);
}

template<class T>
class StateParametersProxy : public StateParameters
{
public:
    StateParametersProxy(const SP<T>& parameters)
        : Proxy(parameters)
    {}

    SP<T> Proxy;

    const auto& GetImmutableData() const { return Proxy->GetImmutableData(); }
    const auto& GetImmutable() const { return Proxy->GetImmutable(); }
};

template<class T> using StateParametersProxyPtr = SP<StateParametersProxy<T>>;

template<class T>
SP<StateParametersProxy<T>> StateParametersProxyCreate(const char* cdl, const SP<T>& stateParameter, const StateParameters::ChainOptions& params, const StateParameters::ChainConnectionOptions& connectionParams, const std::function<void (LocalPropertyErrorsModel&)>& errorsInitializer)
{
    auto result = ::make_shared<StateParametersProxy<T>>(stateParameter);
    if(errorsInitializer == nullptr) {
        result->SetChained(params);
    } else {
        result->SetChained(const_cast<SPCO&>(params).AddInjectedErrors());
        errorsInitializer(result->GetInjectedErrors());
    }
    result->ConnectChainAsProxyOf(cdl, stateParameter, connectionParams);
    return result;
}

class StateParametersGroup
{
public:
    using FForeachHandler = std::function<void (const SP<StateParameters>)>;

    template<class T>
    T Cloned(const std::function<void (T&)>& handler) const
    {
        T result = *reinterpret_cast<const T*>(this);
        handler(result);
        return result;
    }

    template<class T>
    void ForeachT(const std::function<void (const SP<T>&)>& handler) const
    {
        Foreach([&](const SP<StateParameters>& param) {
            handler(param.Cast<T>());
        });
    }

    template<class T>
    void ForeachT(const std::function<void (SP<T>&)>& handler)
    {
        Foreach([&](const SP<StateParameters>& param) {
            handler(const_cast<SP<StateParameters>&>(param).Cast<T>());
        });
    }

    virtual void Foreach(const FForeachHandler& handler) const = 0;
};

template<class T>
class StateParametersContainerGroup : public StateParametersGroup
{
public:
    QVector<SP<T>> Container;

    void ForeachT(const std::function<void (const SP<T>&)>& handler) const
    {
        for(const auto& param : Container) {
            handler(param);
        }
    }

    void Foreach(const std::function<void (const SP<StateParameters>)>& handler) const override
    {
        for(const auto& param : Container) {
            handler(param);
        }
    }
};

template<class T, typename FHandler>
inline T StateParametersGroupCloned(const T& target, const FHandler& handler)
{
    return target.template Cloned<T>(handler);
}

class IStateParameterBase
{
public:
    IStateParameterBase(StateParameters* params);

    virtual bool IsInitialized() const = 0;

protected:
    friend class StateParameters;
    virtual void initialize(){}
};

template<class T>
class StateParameterBase : public IStateParameterBase
{
    using Super = IStateParameterBase;
public:
    template<typename ... Args>
    StateParameterBase(StateParameters* params, const FAction& locker, const FAction& unlocker, Args ... args)
        : Super(params)
        , InputValue(args...)
        , m_parameters(params)
    {
        params->IsLocked.Connect(CONNECTION_DEBUG_LOCATION, [locker, unlocker](bool locked){
            if(!locked) {
                unlocker();
            } else {
                locker();
            }
        });
        adapters::ResetThread(params->IsLocked);
    }

    bool IsInitialized() const override { return m_initializer == nullptr; }
    StateParameters* GetParameters() const { return m_parameters; }

    T InputValue;

private:
    void initialize() override final
    {
        Q_ASSERT(m_initializer != nullptr);
        m_initializer();
        m_initializer = nullptr;
    }

protected:
    StateParameters* m_parameters;
    FAction m_initializer;
};

template<class T>
class StateParameterProperty : public StateParameterBase<T>
{
    using Super = StateParameterBase<T>;
public:
    using value_type = typename T::value_type;
    template<typename ... Args>
    StateParameterProperty(StateParameters* params, const Args&... args)
        : Super(params,
                [this]{ m_connections.clear(); },
                [this]{ m_immutableValue.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue).MakeSafe(m_connections); },
                args...)
        , m_setter([this](const value_type& value){
                    Q_ASSERT(!Super::m_parameters->IsLocked);
                    Super::InputValue.SetSilent(value);
                })
    {
        Super::m_initializer = [this]{
            m_setter = [this](const value_type& value){
                THREAD_ASSERT_IS_MAIN();
                Super::InputValue = value;
            };

            m_immutableValue.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue).MakeSafe(m_connections);
            auto handler = [this]{
                if(Super::m_parameters->IsLocked) {
                    Super::m_parameters->Reset();
                } else {
                    Super::m_parameters->OnChanged.Invoke();
                }
            };
            Super::InputValue.ConnectAction(CONNECTION_DEBUG_LOCATION, handler);
            handler();
        };
    }

    StateParameterProperty& operator=(const value_type& value)
    {
        m_setter(value);
        return *this;
    }

    operator T&() { THREAD_ASSERT_IS_MAIN(); return Super::InputValue; }
    const T& GetImmutableData() const { return m_immutableValue; }
    const auto& GetImmutable() const { return m_immutableValue.Native(); }
    value_type GetImmutable(const value_type& v) const { return m_immutableValue.ValueOr(v); }

private:
    template<class T2> friend struct Serializer;
    template<class T2> friend struct SerializerXml;
    T m_immutableValue;
    DispatcherConnectionsSafe m_connections;
    std::function<void (const value_type&)> m_setter;
};

template<class T> class StateParametersContainer;

template<class T>
class StateParameterImmutableData : public StateParameterBase<LocalPropertySharedPtr<T>>
{
    using Super = StateParameterBase<LocalPropertySharedPtr<T>>;
public:
    using TPtr = T;
    StateParameterImmutableData(StateParameters* params)
        : Super(params, [this]{
            if(Super::InputValue != nullptr) {
                m_lockedModel = Super::InputValue;
                m_lockedModel->Lock();
            }
        }, [this]{
            if(m_lockedModel != nullptr) {
                m_lockedModel->Unlock();
            }
        })
        , m_modelIsValid(false)
        , m_errorId(Error::SP_IncompleteData)
    {
        Super::m_initializer = [this]{
            Super::InputValue.OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this]{
                m_modelConnections.clear();
                if(Super::InputValue != nullptr) {
                    m_modelIsValid.ConnectFrom(CONNECTION_DEBUG_LOCATION, Super::InputValue->IsValid).MakeSafe(m_modelConnections);
                } else {
                    m_modelIsValid = false;
                }
                Super::m_parameters->OnChanged.Invoke();
            });

            auto* cd = Super::m_parameters->GetChainData();
            if(cd != nullptr) {
                if(cd->Include == nullptr) {
                    cd->Errors->Register(CDL, m_errorId, [](bool valid) {
                        return valid;
                    }, m_modelIsValid);
                    Super::m_parameters->IsValid.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_modelIsValid);
                } else {
                    cd->Errors->Register(CDL, m_errorId, [](bool valid, bool included) {
                        return !included || valid;
                    }, m_modelIsValid, cd->Include->InputValue);
                    Super::m_parameters->IsValid.ConnectFromProperties(CONNECTION_DEBUG_LOCATION, [](bool include, bool valid) {
                        return !include || valid;
                    }, cd->Include->InputValue, m_modelIsValid);
                }
            } else {
                Super::m_parameters->IsValid.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_modelIsValid);
            }
        };
    }

    void SetErrorId(const Name& id) { m_errorId = id; }

private:
    friend class StateParametersContainer<T>;
    SharedPointer<T> m_lockedModel;
    LocalPropertyBool m_modelIsValid;
    DispatcherConnectionsSafe m_modelConnections;
    Name m_errorId;
};

enum InitializationWithLock {
    Lock
};

template<class T>
class StateParametersContainer : public StateParameters
{
    template <typename U, typename = void>
    struct has_model_type : std::false_type {};

    template <typename U>
    struct has_model_type<U, std::void_t<typename U::model_type>> : std::true_type {};

    using Super = StateParameters;
public:
    using container_type = typename T::container_type;
    using TPtr = SharedPointer<T>;
    StateParametersContainer()
        : Super(false)
        , m_parameter(this)
    {
    }
    StateParametersContainer(const TPtr& initial)
        : Super(false)
        , m_parameter(this)
    {
        SetParameter(initial);
    }
    StateParametersContainer(const TPtr& initial, InitializationWithLock)
        : Super(false)
        , m_parameter(this)
    {
        SetLockedParameter(initial);
    }

    void SetId(const Name& id)
    {
        SetErrorId(id);
#ifdef QT_DEBUG
        Id = id.AsString();
#endif
    }

    void SetErrorId(const Name& id)
    {
        m_parameter.SetErrorId(id);
    }

    bool HasValue() const
    {
        return !IsNull();
    }

    bool IsNull() const
    {
        return m_parameter.InputValue == nullptr;
    }

    template<class T2>
    void SetParameter(const T2& data)
    {
        m_parameter.InputValue = data;
        adapters::ResetThread(GetProperty());
    }

    void SetChainedParameter(const char* cdl, const SP<StateParametersContainer<T>>& another, const ChainConnectionOptions& options = ChainConnectionOptions())
    {
        SetParameter(another->GetInputData());
        if(another->GetInputData() == nullptr) {
            DisconnectChain();
        } else {
            ConnectChain(cdl, const_cast<ChainConnectionOptions&>(options).AddCapturers(another->GetCapturer()), another);
        }
    }

    void SetChainedParameter(const char* cdl, const SP<T>& another, const ChainConnectionOptions& options = ChainConnectionOptions())
    {
        SetParameter(another);
        if(another == nullptr) {
            DisconnectChain();
        } else {
            ConnectChain(cdl, const_cast<ChainConnectionOptions&>(options).AddCapturers(another->GetCapturer()));
        }
    }

    template<class T2>
    void SetLockedParameter(const T2& data)
    {
#ifdef QT_DEBUG
        m_lockConnections.clear();
#endif
        SetParameter(data);
#ifdef QT_DEBUG
        GetProperty().OnChanged.Connect(CONNECTION_DEBUG_LOCATION, []{
            Q_ASSERT(false);
        }).MakeSafe(m_lockConnections);
        adapters::ResetThread(GetProperty());
#endif
    }

    template<class T2>
    void SetLockedParameter(const T2& data, const ChainOptions& options)
    {
        SetLockedParameter(data);
        SetChained(options);
    }

    const TPtr& GetImmutableData() const
    {
        THREAD_ASSERT_IS_NOT_MAIN()
        return m_parameter.m_lockedModel;
    }

    const container_type& GetImmutable() const
    {
        return GetImmutableData()->GetData()->Native();
    }

    template<typename ... Dispatchers>
    DispatcherConnections AddValidatorOnChanged(const char* connectionInfo, const std::function<bool (const T& value)>& validator, const Dispatchers&... args)
    {
        auto dispatcher = GetProperty().DispatcherParamsOnChanged(CONNECTION_DEBUG_LOCATION).CreateDispatcher();
        return IsValid.ConnectFromDispatchers(connectionInfo, [this, validator, dispatcher]{
            if(GetProperty() == nullptr) {
                return false;
            }
            return validator(*GetProperty());
        }, *dispatcher, args...);
    }

    template<typename ... Dispatchers>
    DispatcherConnections AddValidator(const char* connectionInfo, const std::function<bool (const T& value)>& validator, const Dispatchers&... args)
    {
        return IsValid.ConnectFromDispatchers(connectionInfo, [this, validator]{
            if(GetProperty() == nullptr) {
                return false;
            }
            return validator(*GetProperty());
        }, args...);
    }

    LocalPropertySharedPtr<T>& GetProperty()
    {
        return m_parameter.InputValue;
    }

    const TPtr& GetInputData() const
    {
        THREAD_ASSERT_IS_MAIN()
        return m_parameter.InputValue;
    }

    template <typename U = T, typename = typename std::enable_if<has_model_type<U>::value>::type>
    const SP<typename U::model_type>& GetInputDataData() const
    {
        THREAD_ASSERT_IS_MAIN()
        return m_parameter.InputValue->GetData();
    }

    const container_type& GetInput() const
    {
        return GetInputData()->GetData()->Native();
    }

private:
    std::function<SmartPointerWatcherPtr()> m_captureHandler;
    StateParameterImmutableData<T> m_parameter;
#ifdef QT_DEBUG
    DispatcherConnectionsSafe m_lockConnections;
#endif
};

template<class T> using StateParametersContainerPtr = SharedPointer<StateParametersContainer<T>>;
template<class T> using StateParametersContainerPtrInitialized = SharedPointerInitialized<StateParametersContainer<T>>;

inline uint qHash(const SharedPointer<StateParameters>& key, uint seed = 0) { return qHash(key.get(), seed); }

template<class T>
class StateCalculatorSwitcher
{
    template<class T2> friend class StateCalculator;
    StateCalculatorSwitcher(T* calculator)
        : m_pointer([calculator]{
            calculator->Enabled = true;
        }, [calculator]{
            calculator->Enabled = false;
        })
    {}
public:
    SmartPointerWatcherPtr Capture() { return m_pointer.Capture(); }
    SmartPointer* GetCapturer() { return &m_pointer; }

private:
    SmartPointer m_pointer;
};

template<class T>
class StateCalculator : public ThreadCalculator<T>
{
    using Super = ThreadCalculator<T>;
public:
    StateCalculator()
        : Super(ThreadHandlerMain)
        , Enabled(false)
        , Valid(false)
        , m_dependenciesAreUpToDate(true)
    {
        m_onChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_dependenciesAreUpToDate);

        Enabled.OnChanged += {this, [this]{
            THREAD_ASSERT_IS_MAIN();
            if(Enabled) {
                m_onChanged.OnDirectChanged += { this, [this]{
                    if(m_interruptor != nullptr) {
                        m_interruptor->Interrupt();
                    }
                    Super::Cancel();
                    if(!Valid) {
                        return;
                    }
                    Q_ASSERT_X(m_calculator && m_preparator && m_releaser, __FUNCTION__, m_calculatorProblemLocation);
                    Valid.SetState(false);
                }};
                m_onChanged += { this, [this]{
                    Valid.SetState(false);

                    DEBUG_PRINT_INFO_ACTION(this,
                        qDebug() << m_dependenciesAreUpToDate << (m_dependenciesAreUpToDate ? QString() : m_dependenciesAreUpToDate.ToString());
                        StringBuilder paramsDebug;
                        for(const auto& parameter : m_stateParameters) {
                            if(!parameter->IsValid) {
                                paramsDebug.Add(",", parameter->Id.isEmpty() ? "Unnamed" : parameter->Id);
                            }
                        }
                        qDebug() << "Invalid Parameters:" << paramsDebug;
                    );

//                    Q_ASSERT_X(m_calculator != nullptr, __FUNCTION__, m_calculatorProblemLocation);
                    if(m_calculator == nullptr) {
                        return;
                    }

                    if(m_dependenciesAreUpToDate) {
                        Calculate(m_calculator, m_preparator, m_releaser);
                    } else {
                        OnCalculationRejected();
                    }
                }};
                RequestRecalculate();
            } else {
                m_onChanged -= this;
                m_onChanged.OnDirectChanged -= this;
                Valid.SetState(false);
            }
        }};

        adapters::ResetThread(m_dependenciesAreUpToDate.AsProperty(), Enabled);
    }

    ~StateCalculator()
    {
        if(m_interruptor != nullptr) {
            m_interruptor->Interrupt();
        }
    }

    Interruptor GetInterruptor()
    {
        if(m_interruptor == nullptr) {
            m_interruptor = new Interruptor();
            OnAboutToCalculate.Connect(CONNECTION_DEBUG_LOCATION, [this] {
                m_interruptor->Reset();
            });
        }
        return *m_interruptor;
    }

    SmartPointerWatcherPtr Capture() const
    {
        if(m_switcher == nullptr) {
            m_switcher = new StateCalculatorSwitcher<StateCalculator>(const_cast<StateCalculator*>(this));
        }
        return m_switcher->Capture();
    }

    SmartPointer* GetCapturer() const
    {
        if(m_switcher == nullptr) {
            m_switcher = new StateCalculatorSwitcher<StateCalculator>(const_cast<StateCalculator*>(this));
        }
        return m_switcher->GetCapturer();
    }

    void RequestRecalculate() const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.Update();
    }

    void Cancel()
    {
        m_onChanged.OnDirectChanged();
    }

    void Disconnect(const char* cdl, const std::function<T ()>& calculator = nullptr)
    {
        DisconnectWithoutCalculatorReset();
#ifdef QT_DEBUG
        m_calculatorProblemLocation = cdl;
#endif
        SetCalculator(calculator);
    }

    // Do not this function. It's legacy
    void DisconnectWithoutCalculatorReset()
    {
        THREAD_ASSERT_IS_MAIN();
        m_connections.clear();
        m_dependenciesAreUpToDate.Reset(false);
        m_stateParameters.clear();
        Cancel();
    }

    void SetCalculator(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{},
                       const typename ThreadCalculatorData<T>::Releaser& releaser = []{})
    {
        m_preparator = preparator;
        m_calculator = calculator;
        m_releaser = releaser;
    }

    template<typename ... Args, typename Function>
    void SetCalculatorWithParams(const char* connectionInfo, const Function& handler, Args... args)
    {
        Q_ASSERT(m_stateParameters.isEmpty());
        ConnectCombined(connectionInfo, args...);
        SetCalculatorBasedOnStateParameters([=]{
            return handler(args...);
        });
    }

    void SetCalculatorBasedOnStateParameters(const typename ThreadCalculatorData<T>::Calculator& calculator)
    {
        auto params = m_stateParameters;
        m_preparator = [params]{
            for(const auto& parameters : params) {
                parameters->Lock();
            }
        };
        m_releaser = [params]{
            for(const auto& parameters : params) {
                parameters->Unlock();
            }
        };
        m_calculator = calculator;
    }

    template<class DoubleBuffer>
    void InitializeByDoubleBuffer(const char* connectionInfo, DoubleBuffer& buffer, bool enable = true, const std::function<bool (const typename DoubleBuffer::container_type&)>& validHandler = nullptr)
    {
        OnCalculationRejected += { this, [&buffer]{
            buffer.EditData()->Clear();
        }};
        Super::OnCalculated += { this, [validHandler, &buffer](const auto& container){
            buffer.EditData()->Set(container);
            buffer.EditData()->IsValid.SetState(validHandler == nullptr ? true : validHandler(container));
        }};
        buffer.EditData()->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
        Enabled = enable;
        buffer.Enabled = true;
    }

    template<class DoubleBuffer>
    void ConnectDoubleBuffer(const char* connectionInfo, DoubleBuffer& buffer)
    {
        OnCalculationRejected.Connect(connectionInfo, [&buffer]{
            buffer.EditData()->Clear();
        });
        buffer.EditData()->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
        buffer.Enabled = true;
    }

    template<class Buffer>
    void InitializeByBuffer(const char* connectionInfo, Buffer& buffer, bool enable = true)
    {
//        OnCalculationRejected += { this, [&buffer]{
//            buffer->Clear();
//        }};
        Super::OnCalculated += { this, [&buffer](const auto& container){
            buffer->Set(container);
            buffer->IsValid.SetState(true);
        }};
        buffer->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
        Enabled = enable;
    }

    template<class Buffer>
    void ConnectBuffer(const char* connectionInfo, Buffer& buffer)
    {
//        OnCalculationRejected.Connect(connectionInfo, [&buffer]{
//            buffer->Clear();
//        });
        buffer->IsValid.ConnectFromStateProperty(connectionInfo, Valid);
    }

    const StateCalculator& Connect(const char* cdl, const LocalPropertyErrorsModel& errors) const
    {
        THREAD_ASSERT_IS_MAIN();
        return Connect(cdl, errors.IsValid);
    }

    const StateCalculator& Connect(const char* cdl, const StateParametersGroup& wrapper) const
    {
        THREAD_ASSERT_IS_MAIN();
        wrapper.Foreach([&](const auto& param) {
            Connect(cdl, param);
        });
        return *this;
    }

    template<class T2>
    const StateCalculator& Connect(const char* connection, const StateCalculator<T2>& calculator) const
    {
        THREAD_ASSERT_IS_MAIN();
        return Connect(connection, calculator.Valid);
    }

    template<class T2>
    const StateCalculator& Connect(const char* connection, const LocalProperty<T2>& property) const
    {
        THREAD_ASSERT_IS_MAIN();
        auto& nonConstProperty = const_cast<LocalProperty<T2>&>(property);
        m_onChanged.ConnectFrom(connection, nonConstProperty.OnChanged).MakeSafe(m_connections);
        return *this;
    }

    template<typename ... Args>
    const StateCalculator& ConnectCombined(const char* connection, const Args&... args) const
    {
        (Connect(connection, args), ...);
        return *this;
    }

    template<class T2>
    const StateCalculator& Connect(const char* connection, const SharedPointer<StateParametersProxy<T2>>& params) const
    {
        Connect(connection, params.template Cast<StateParameters>());
        const auto& proxy = params->Proxy;
        if(!proxy->IsInitialized()) {
            proxy->Initialize();
        }
        Connect(connection, proxy->OnChanged);
        Connect(connection, proxy->m_isValid);
        m_stateParameters.insert(proxy);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const SharedPointer<StateParameters>& params) const
    {
        if(!params->IsInitialized()) {
            params->Initialize();
        }
        Connect(connection, params->OnChanged);
        Connect(connection, params->m_isValid);
        Connect(connection, params->IsValid);
        m_stateParameters.insert(params);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const StatePropertyBoolCommutator& dispatcher) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.ConnectFrom(connection, dispatcher).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const StateProperty& dispatcher) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_dependenciesAreUpToDate.ConnectFrom(connection, dispatcher).MakeSafe(m_connections);
        return *this;
    }

    const StateCalculator& Connect(const char* connection, const Dispatcher& onChanged) const
    {
        THREAD_ASSERT_IS_MAIN();
        m_onChanged.ConnectFrom(connection, onChanged).MakeSafe(m_connections);
        return *this;
    }

    const StatePropertyBoolCommutator& GetDependenciesState() const { return m_dependenciesAreUpToDate; }

    mutable LocalPropertyBool Enabled;
    StateProperty Valid;
    Dispatcher OnCalculationRejected;
    CommonDispatcher<const Exception&> OnExceptionCaught;

protected:
    void onPreRecalculate() override
    {
        if(m_calculator != nullptr) {
            OnCalculationRejected();
        }
    }
    void onPostRecalculate() override
    {
        Valid.SetState(true);
    }
    bool acceptResult() override
    {
        return m_calculator != nullptr && m_dependenciesAreUpToDate;
    }

    void onExceptionCaught(const std::exception_ptr& e) override
    {
        OnCalculationRejected();
        try {
            std::rethrow_exception(e);
        }  catch (const Exception& e) {
            OnExceptionCaught(e);
        }
    }

private:
    void Calculate(const typename ThreadCalculatorData<T>::Calculator& calculator, const typename ThreadCalculatorData<T>::Preparator& preparator = []{},
                   const typename ThreadCalculatorData<T>::Releaser& releaser = []{})
    {
        Super::Calculate(calculator, preparator, releaser);
    }

private:
    typename ThreadCalculatorData<T>::Calculator m_calculator;
    typename ThreadCalculatorData<T>::Preparator m_preparator;
    typename ThreadCalculatorData<T>::Releaser m_releaser;
    mutable StatePropertyBoolCommutator m_dependenciesAreUpToDate;
    mutable DispatcherConnectionsSafe m_connections;
    mutable DispatchersCommutatorWithDirect m_onChanged;
    mutable QSet<SharedPointer<StateParameters>> m_stateParameters;
    mutable ScopedPointer<StateCalculatorSwitcher<StateCalculator>> m_switcher;
    ScopedPointer<Interruptor> m_interruptor;
#ifdef QT_DEBUG
    const char* m_calculatorProblemLocation = "initialization";
#endif
};

class StateParametersChainData
{
public:
    using FCapture = std::function<void (StateParametersChainData&)>;
    StateParametersChainData(StateParameters* params, const FCapture& capture, const FCapture& release);

    const LocalPropertyErrorsModel& GetErrors() const { return *Errors; }
    LocalPropertyErrorsModel& EditErrors() { return *Errors; }
    const LocalPropertyErrorsModel& GetProcesses() const { return *Processes; }
    StateParameterProperty<LocalPropertyBool>* GetInclude() const { return Include.get(); }
    SmartPointerWatcherPtr Capture() const { return m_includer.Capture(); }
    SmartPointer* GetCapturer() const { return &m_includer; }
    void SetCaptureHandler(const FCapture& capture);

private:
    friend class StateParameters;
    template<class T> friend class StateParameterImmutableData;
    SP<LocalPropertyErrorsModel> Errors;
    SP<LocalPropertyErrorsModel> Processes;
    SP<StateParameterProperty<LocalPropertyBool>> Include;
    SmartPointerWatchers Captures;
    FCapture m_capture;
    FCapture m_release;
    mutable SmartPointer m_includer;
};

template<class T>
class StateImmutableData {
    using TPtr = SharedPointer<T>;
public:
    using model_type = T;
    using container_type = typename T::container_type;
    using FHandler = std::function<container_type ()>;
    StateImmutableData(const TPtr& data = ::make_shared<T>())
        : m_lockCounter(0)
        , m_isDirty(false)
#ifndef QT_NO_DEBUG
        , m_internalEditing(false)
#endif
        , Enabled(m_calculator.Enabled)
        , IsValid(data->IsValid)
    {
        m_calculator.OnCalculated += { this, [this](bool){
            if(m_lockCounter != 0) {
                m_isDirty = true;
                if(m_lockCounter == 0) {
                    m_calculator.RequestRecalculate();
                }
                return;
            }

#ifndef QT_NO_DEBUG
        guards::LambdaGuard guard([this]{ m_internalEditing = false; }, [this]{ m_internalEditing = true; });
#endif 
            if(m_handler != nullptr) {
                auto data = m_handler();
                m_data->Swap(data);
            } else {
                m_data->Clear();
            }
            m_data->IsValid.SetState(true);
        }};

//        m_calculator.OnCalculationRejected += { this, [this]{
//            if(m_lockCounter == 0) {
//    #ifndef QT_NO_DEBUG
//                guards::LambdaGuard guard([this]{ m_internalEditing = false; }, [this]{ m_internalEditing = true; });
//    #endif
//                m_data->Clear();
//            }
//        }};

        m_data = data;
#ifndef QT_NO_DEBUG
        m_data->OnChanged += { this, [this]{
            Q_ASSERT(m_internalEditing);
        }};
#endif
        m_data->IsValid.ConnectFromStateProperty(CONNECTION_DEBUG_LOCATION, m_calculator.Valid);

        adapters::SetThreadSafe(m_data->IsValid, m_data->OnChanged);
        adapters::ResetThread(m_calculator.Valid, m_calculator.OnCalculated);
    }

    guards::LambdaGuardPtr CreateLocker()
    {
        THREAD_ASSERT_IS_NOT_MAIN()
        return ::make_shared<guards::LambdaGuard>([this]{
            ThreadsBase::DoMainAwait(CONNECTION_DEBUG_LOCATION,[this]{ Unlock(); });
        }, [this]{
            ThreadsBase::DoMainAwait(CONNECTION_DEBUG_LOCATION,[this]{ Lock(); });
        });
    }

    void Lock() const
    {
        THREAD_ASSERT_IS_MAIN();
        ++m_lockCounter;
        DEBUG_PRINT_INFO_ACTION(this,
            qDebug() << this << "Locked";
        );
        Q_ASSERT(m_lockCounter >= 0);
    }

    void Unlock() const
    {
        --m_lockCounter;
        if(m_lockCounter == 0 && m_isDirty) {
            m_isDirty = false;
            m_calculator.RequestRecalculate();
        }
        DEBUG_PRINT_INFO_ACTION(this,
            qDebug() << this << "UnLocked";
        );
        Q_ASSERT(m_lockCounter >= 0);
    }

    void AttachSwap(const TPtr& data)
    {
        AttachSource(data, [data]{
            container_type result;
            result.swap(const_cast<container_type&>(data->Native()));
            return result;
        });
    }

    void AttachEmpty()
    {
        m_calculator.Disconnect(CDL);
        m_handler = []{
            return container_type();
        };
        m_calculator.SetCalculator([]{
            return true;
        });
        m_calculator.RequestRecalculate();
    }

    void AttachCopy(const TPtr& externalData)
    {
        if(externalData == nullptr) {
            AttachSource(nullptr);
            return;
        }
        AttachSource<T>(externalData, [externalData]{ return externalData->Clone(); });
    }

    void AttachSource(const std::function<void (StateCalculator<bool>&)>& connectorHandler, const FHandler& handler = nullptr)
    {
        m_calculator.Disconnect(CDL);
//        Q_ASSERT(handler != nullptr);
        m_handler = handler;
        if(m_handler){
            connectorHandler(m_calculator);

            m_calculator.SetCalculator([]{
                return true;
            });

            m_calculator.RequestRecalculate();
        }        
    }

    template<class ExternalData>
    void AttachSource(const SharedPointer<ExternalData>& externalData, const FHandler& handler)
    {
        if(externalData == nullptr) {
            AttachSource(nullptr);
            return;
        }

        AttachSource([externalData](StateCalculator<bool>& calculator){
            calculator.Connect(CONNECTION_DEBUG_LOCATION, externalData->IsValid)
                    .Connect(CONNECTION_DEBUG_LOCATION, externalData->OnChanged);
        }, handler);
    }

    void Disconnect(const char* location) {
        m_calculator.Disconnect(location);
    }

    const TPtr& GetData() const { return m_data; }
    qint32 GetLockCounter() const { return m_lockCounter; }
    const StateCalculator<bool>& GetCalculator() const { return m_calculator; }
    SmartPointer* GetCapturer() { return m_calculator.GetCapturer(); }

protected:
    TPtr m_data;
    FHandler m_handler;
    StateCalculator<bool> m_calculator;
    mutable std::atomic_int m_lockCounter;
    mutable std::atomic_bool m_isDirty;
#ifndef QT_NO_DEBUG
    bool m_internalEditing;
#endif

public:
    LocalPropertyBool& Enabled;
    StateProperty& IsValid;
};

template<class T> using StateParametersImmutableData = StateParametersContainer<StateImmutableData<T>>;
template<class T> using StateImmutableDataPtr = SharedPointer<StateImmutableData<T>>;

template<class T>
struct DefaultImmutableData
{
    static const StateImmutableDataPtr<T> Value;
};

#define IMPLEMENT_DEFAULT_IMMUTABLE_DATA(T) \
const StateImmutableDataPtr<T> DefaultImmutableData<T>::Value = []{ \
        auto result = ::make_shared<StateImmutableData<T>>(::make_shared<T>()); \
        result->IsValid.SetState(true); \
        return result; \
}();

template<class T, class T2, typename TPtr = SharedPointer<T>>
SharedPointer<StateParametersImmutableData<T>> StateParametersImmutableDataCreate(const SharedPointer<T2>& source, const std::function<void (StateCalculator<bool>&)>& connectorHandler, const typename StateImmutableData<T>::FHandler& handler = nullptr)
{
    auto result = ::make_shared<StateParametersImmutableData<T>>();
    result->InputValue = ::make_shared<T>();
    result->InputValue->AttachSource(source, connectorHandler, handler);
    result->InputValue = true;
    return result;
}

template<class T, class T2, typename TPtr = SharedPointer<T>>
SharedPointer<StateParametersImmutableData<T>> StateParametersImmutableDataCreate(const SharedPointer<T2>& source, const typename StateImmutableData<T>::FHandler& handler = nullptr)
{
    auto result = ::make_shared<StateParametersImmutableData<T>>();
    result->InputValue = ::make_shared<T>();
    result->InputValue->AttachSource(source, handler);
    result->InputValue = true;
    return result;
}

template<class T>
class IStateImmutableData
{
public:
    using TPtr = SharedPointer<T>;

    virtual void Swap(const TPtr& data) = 0;
    virtual TPtr Clone() const = 0;
    virtual void Clear() = 0;

    StateProperty IsValid;
    Dispatcher OnChanged;
};

template<class T>
class StateDoubleBufferData
{
public:
    using TPtr = SharedPointer<T>;
    using container_type = typename T::container_type;
    StateDoubleBufferData(bool copy = false)
        : StateDoubleBufferData(::make_shared<T>(), ::make_shared<T>(), copy)
    {}

    StateDoubleBufferData(const std::function<container_type (const container_type& c)>& handler)
        : m_immutableData(::make_shared<StateImmutableData<T>>(::make_shared<T>()))
        , m_data(::make_shared<T>())
        , Enabled(m_immutableData->Enabled)
    {
        Enabled.OnChanged += { this, [this, handler]{
            if(Enabled) {
                m_immutableData->AttachSource(m_data, [this, handler]{
                    return handler(m_data->Native());
                });
            } else {
                m_immutableData->AttachCopy(nullptr);
            }
        }};
        adapters::ResetThread(Enabled);
    }

    using TPtr = SharedPointer<T>;
    StateDoubleBufferData(const TPtr& source, const TPtr& immutable, bool copy = false)
        : m_immutableData(::make_shared<StateImmutableData<T>>(immutable))
        , m_data(source)
        , Enabled(m_immutableData->Enabled)
    {
        Enabled.OnChanged += { this, [this, source, copy]{
            if(Enabled) {
                if(copy) {
                    m_immutableData->AttachCopy(source);
                } else {
                    m_immutableData->AttachSwap(source);
                }
            } else {
                m_immutableData->AttachCopy(nullptr);
            }
        }};
        adapters::ResetThread(Enabled);
    }

    void Reset()
    {
        m_data->IsValid.SetState(false);
    }

    void Set(const typename T::container_type& data)
    {
        m_data->Set(data);
        m_data->IsValid.SetState(true);
    }

    const TPtr& EditData() { return m_data; }
    const TPtr& EditData() const { return m_data; }
    const StateImmutableDataPtr<T>& GetImmutableData() const { return m_immutableData; }
    SmartPointer* GetCapturer() { return m_immutableData->GetCapturer(); }

private:
    StateImmutableDataPtr<T> m_immutableData;
    TPtr m_data;

public:
    LocalPropertyBool& Enabled;
};

class StateUpdateObjectResult
{
public:
    static AsyncResult GenerateUpdateResult(const char* location, const StateProperty* valid, Dispatcher* onDeleted)
    {
        AsyncResult updatedResult;
        auto* nonConst = const_cast<StateProperty*>(valid);
        if(*valid) {
            nonConst->OnChanged.OnFirstInvoke([location, updatedResult, valid, onDeleted]{
                generateUpdateResult(location, valid, onDeleted, updatedResult);
            });
        } else {
            generateUpdateResult(location, valid, onDeleted, updatedResult);
        }
        return updatedResult;
    }

private:
    static void generateUpdateResult(const char* location, const StateProperty* valid, Dispatcher* onDeleted, const AsyncResult& updatedResult)
    {
        auto connections = ::make_shared<DispatcherConnectionsSafe>();
        auto* nonConst = const_cast<StateProperty*>(valid);
        nonConst->OnChanged.OnFirstInvoke([updatedResult, connections]{
            connections->clear();
            updatedResult.Resolve(true);
        });
        onDeleted->Connect(location, [connections, updatedResult]{
            connections->clear();
            updatedResult.Resolve(false);
        }).MakeSafe(*connections);
    }
};

template<typename... Dispatchers>
SharedPointer<StateProperty> StatePropertyCreateFromDispatchers(const char* connection, const std::function<bool ()>& handler, Dispatcher& dispatcher, Dispatchers&... dispatchers)
{
    auto result = ::make_shared<WithDispatcherConnectionsSafe<StateProperty>>();
    result->ConnectFromDispatchers(connection, handler, dispatcher, dispatchers...).MakeSafe(result->Connections);
    return result;
}

template<typename... Props, typename FHandler>
SharedPointer<StateProperty> StatePropertyCreate(const char* connection, const FHandler& handler, Props&... properties)
{
    auto result = ::make_shared<WithDispatcherConnectionsSafe<StateProperty>>();
    result->ConnectFrom(connection, handler, properties...).MakeSafe(result->Connections);
    return std::move(result);
}

class DispatcherConnectionChain
{
public:
    DispatcherConnectionChain();

public:
    using FDep = std::function<const Dispatcher* ()>;
    template<typename FGetter>
    struct PropertyWrapper
    {
        explicit PropertyWrapper(const FGetter& getter)
            : Getter(getter)
        {}

        FGetter Getter;
    };

    template<class Property, class ... Deps>
    DispatcherConnectionChain(const char* cdl, const Property& property, const Deps&... deps)
        : DispatcherConnectionChain(cdl, property.OnChanged, FDep([&property]() -> const Dispatcher* {
            return property == nullptr ? nullptr : DefaultDispatcher();
        }), deps...)
    {
    }

    template<class ... Deps>
    DispatcherConnectionChain(const char* cdl, const Dispatcher& dispatcher, const FDep& dep, const Deps&... deps)
    {
        add(cdl, dep);
        adapters::Combine([&](const auto& dep) {
            add(cdl, dep);
        }, deps...);
        dispatcher.ConnectAndCall(CDL, [this]{
            update();
        }).MakeSafe(m_connections);
    }

    template<class ... Deps>
    SP<DispatcherConnectionChain> CreateSubChain(const char* cdl, const Deps&... deps)
    {
        auto result = ::make_shared<DispatcherConnectionChain>();
        result->connectFrom(cdl, *this);
        adapters::Combine([&](const auto& dep) {
            result->add(cdl, dep);
        }, deps...);
        result->update();
        return result;
    }
    void ConnectFrom(const char* cdl, const DispatcherConnectionChain& another);

    StateProperty IsValid;

    DispatcherConnection OnFailed(const char* cdl, const FAction& action);
    DispatcherConnection OnValid(const char* cdl, const FAction& action);

    static const Dispatcher* DefaultDispatcher();
    template<class FGetter>
    static PropertyWrapper<FGetter> CreatePropertyWrapper(const FGetter& getter) { return PropertyWrapper<FGetter>(getter); }

private:
    void connectFrom(const char* cdl, const DispatcherConnectionChain& another);
    void invalidResult();
    void validResult();
    void update();
    template<class T>
    void add(const char* cdl, const PropertyWrapper<T>& propertyGetter)
    {
        add(cdl, [propertyGetter]{
            return &propertyGetter.Getter()->OnChanged;
        });
        add(cdl, [propertyGetter]() -> const Dispatcher* {
            return *propertyGetter.Getter() == nullptr ? nullptr : DefaultDispatcher();
        });
    }
    void add(const char* cdl, const FDep& dependency);
    void add(const char* cdl, const SP<DispatcherConnectionChain>& another) { connectFrom(cdl, *another); }

private:
    QVector<FDep> m_deps;
    DispatcherConnectionsSafe m_connections;
    DispatcherConnectionsSafe m_depConnections;
};

using DispatcherConnectionChainPtr = SP<DispatcherConnectionChain>;

template<typename ... Args>
DispatcherConnectionChainPtr DispatcherConnectionChainCreate(const char* cdl, const Args&... args)
{
    return ::make_shared<DispatcherConnectionChain>(cdl, args...);
}

#endif // STATEPROPERTY_H
