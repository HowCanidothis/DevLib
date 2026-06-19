#include "stateproperty.h"

#include "localpropertyerrorscontainer.h"

IMPLEMENT_GLOBAL_ERROR(SP_IncompleteData, TRS(etr::tr("Incomplete output")))

void StateProperty::SetState(bool state)
{
    Super::SetValue(state);
}

DispatcherConnections StateProperty::ConnectFromStateProperty(const char* location, const StateProperty& property)
{
    return Super::ConnectFrom(location, [this](bool valid) { return valid ? Super::m_value : valid; }, property);
}

bool StateProperty::Wait(const char* cdl, const QVector<const StateProperty*>& stateProperties, qint32 msecs)
{
    AsyncResult result;
    OnFirstInvokePerformWhenEveryIsValid(cdl, [&] {
        result.Resolve(true);
    }, stateProperties);
    return result.Wait(msecs);
}

DispatcherConnection StateProperty::OnFirstInvokePerformWhenEveryIsValid(const char* location, const FAction& handler, const QVector<const StateProperty*>& stateProperties)
{
    if(stateProperties.isEmpty()) {
        handler();
        return DispatcherConnection();
    }
    auto commutator = ::make_shared<WithDispatcherConnectionsSafe<LocalPropertyBoolCommutator>>(LocalPropertyBoolCommutator::And);
    for(const auto* prop : stateProperties) {
        commutator->ConnectFrom(location, *prop).MakeSafe(commutator->Connections);
    }

    return commutator->AsProperty().OnChanged.OnFirstInvoke([location, handler, commutator]{
        handler();
        ThreadsBase::DoMain(location, [commutator]{}); // Safe deletion
    });
}

StatePropertyBoolCommutator::StatePropertyBoolCommutator(bool defaultState)
    : Super(And)
#ifdef QT_DEBUG
    , OnDirectChanged(m_commutator.OnDirectChanged)
#endif
{
    m_property.EditSilent() = defaultState;
    m_commutator.OnDirectChanged += { this, [this]{
        if(!value()) {
//#ifdef QT_DEBUG
//            m_property.SetValueForceInvoke(false);
//#else
            m_property.SetValue(false);
//#endif
        }
    }};
    adapters::ResetThread(m_commutator.OnDirectChanged);
}

void StatePropertyBoolCommutator::Update()
{
    m_property = false;
    m_commutator.Invoke();
}

void StateParameters::setIncludable()
{
    auto included = SmartPointerWatcherCreate();
    auto* includer = m_chainData->GetCapturer();
    m_chainData->Include->InputValue.ConnectAndCall(CDL, [included, includer](bool include){
        if(include) {
            *included = includer->Capture();
        } else {
            *included = nullptr;
        }
    });
}

void StateParameters::Initialize()
{
    if(m_initializer == nullptr) {
        return;
    }
    m_initializer();
    onInitialized();
    if(m_chainData != nullptr) {
        if(m_chainData->GetInclude() != nullptr) {
            setIncludable();
            IsValid.ConnectFromProperties(CDL, [](bool include, bool valid){
                return !include || valid;
            }, m_chainData->Include->InputValue, m_chainData->Errors->IsValid);
        } else {
            IsValid.ConnectFrom(CDL, m_chainData->Errors->IsValid);
        }
    }
    m_initializer = nullptr;
}

LocalPropertyErrorsModel& StateParameters::GetInjectedErrors()
{
    Q_ASSERT(m_injectedErrors != nullptr);
    return *m_injectedErrors;
}

void StateParameters::SetChained(const ChainOptions& params)
{
#ifdef QT_DEBUG
    Q_ASSERT(m_chainData == nullptr && !m_hasEmptyChainData);
#endif
    if(params.InjectErrors) {
        Q_ASSERT(m_injectedErrors == nullptr);
        m_injectedErrors = new LocalPropertyErrorsModel();
    }

    m_chainData = ::make_shared<StateParametersChainData>(this, [](auto&){}, [](StateParametersChainData& data){
        data.Errors->Clear();
        data.Captures.clear();
        data.Processes->Clear();
    });
    if(params.Includable) {
        m_chainData->Include = ::make_shared<StateParameterProperty<LocalPropertyBool>>(this, params.Included);
    }
//    const_cast<LocalPropertyBool&>(IsValid.AsProperty()).EditSilent() = false;
}

void StateParameters::DisconnectChain()
{
    Q_ASSERT(m_chainData != nullptr);
    m_chainData->SetCaptureHandler([](auto&){});
}

void StateParameters::ConnectChain(const char* cdl, const ChainConnectionOptions& params)
{
    if(m_chainData == nullptr)
    {
#ifdef QT_DEBUG
        Q_ASSERT(!m_hasEmptyChainData);
#endif
        SetChained(SPCO());
    }
    auto errorsToConnect = params.Errors;
    if(m_injectedErrors != nullptr) {
        errorsToConnect.append(m_injectedErrors.get());
    }
    auto capturers = params.Capturers;
    auto processes = params.Processes;
    const auto& processId = params.ProcessId;
    auto captureHandler = [cdl, errorsToConnect, processId, capturers, processes](StateParametersChainData& data){
        if(!processId.IsNull()) {
            data.Processes->Add(processId);
        }
        data.Errors->ConnectFromModels(CDL, errorsToConnect);
        data.Processes->ConnectFromModels(CDL, processes);
        for(const auto& capturer : capturers) {
            data.Captures.append(capturer->Capture());
        }
    };
    m_chainData->SetCaptureHandler(captureHandler);
}

void StateParameters::ConnectChainAsProxyOf(const char* cdl, const SP<StateParameters>& another, const ChainConnectionOptions& options)
{
    auto* cd = another->GetChainData();
    Q_ASSERT(cd != nullptr);

    ConnectChain(cdl, const_cast<ChainConnectionOptions&>(options).AddCapturers(cd->GetCapturer()).AddErrors(cd->GetErrors()).AddProcesses(cd->GetProcesses()));
}

bool StateParameters::IsIncludedImmutable() const
{
    return m_chainData == nullptr || m_chainData->Include == nullptr ? true : m_chainData->Include->GetImmutable();
}

LocalPropertyBool& StateParameters::GetIncluded() const
{
    Q_ASSERT(m_chainData != nullptr && m_chainData->GetInclude() != nullptr);
    return m_chainData->GetInclude()->InputValue;
}

const LocalPropertyErrorsModel& StateParameters::GetErrors() const
{
    Q_ASSERT(m_chainData != nullptr);
    return m_chainData->GetErrors();
}

const LocalPropertyErrorsModel& StateParameters::GetProcesses() const
{
    Q_ASSERT(m_chainData != nullptr);
    return m_chainData->GetProcesses();
}

SmartPointer* StateParameters::GetCapturer() const
{
    Q_ASSERT(m_chainData != nullptr);
    return m_chainData->GetCapturer();
}

IStateParameterBase::IStateParameterBase(StateParameters* params)
{
    params->m_parameters.append(this);
}

StateParameters::StateParameters(bool valid)
    : IsValid(valid)
    , IsLocked(false)
    , m_counter(0)
    , m_isValid(true)
    , m_initializer([this]{
        THREAD_ASSERT_IS_MAIN();
        for(auto* parameter : ::make_const(m_parameters)) {
            if(!parameter->IsInitialized()) {
                parameter->initialize();
            }
        }
    })
{
    DEBUG_SYNC(this, { &IsValid, &m_isValid });
}

void StateParameters::Lock()
{
    ++m_counter;
    IsLocked = m_counter != 0;
}

void StateParameters::Unlock()
{
    --m_counter;
    IsLocked = m_counter != 0;
    if(!IsLocked) {
        m_isValid.SetState(true);
    }
#ifdef QT_DEBUG
    if(m_counter < 0) {
       DEBUG_PRINT_INFO(this);
    }
    Q_ASSERT(m_counter >= 0);
#endif
}

void StateParameters::Reset()
{
    m_isValid.SetState(false);
}

void StateParameters::extract(const SP<StateParameters>& params,
                              QVector<const LocalPropertyErrorsModel*>& errors,
                              QVector<const LocalPropertyErrorsModel*>& processes,
                              QVector<SmartPointer*>& captures)
{
#ifdef QT_DEBUG
    if(params->m_hasEmptyChainData) {
        return;
    }
#endif
    auto* chainData = params->GetChainData();
    if(chainData == nullptr) {
        params->SetEmptyChained();
        return;
    }
    errors.append(chainData->Errors.get());
    processes.append(chainData->Processes.get());
    if(chainData->Include == nullptr) {
        captures.append(chainData->GetCapturer());
    }
}

void StateParameters::extract(const StateParametersGroup& wrapper,
             QVector<const LocalPropertyErrorsModel*>& errors,
             QVector<const LocalPropertyErrorsModel*>& processes,
             QVector<SmartPointer*>& captures)
{
    wrapper.Foreach([&](const SP<StateParameters>& param) {
        extract(param, errors, processes, captures);
    });
}

DispatcherConnection DispatcherConnectionChain::OnFailed(const char* cdl, const FAction& action)
{
    if(!IsValid) {
        action();
    }
    return IsValid.OnChanged.Connect(cdl, [this, action] {
        if(!IsValid) {
            action();
        }
    });
}

DispatcherConnection DispatcherConnectionChain::OnValid(const char* cdl, const FAction& action)
{
    if(IsValid) {
        action();
    }
    return IsValid.OnChanged.Connect(cdl, [this, action] {
        if(IsValid) {
            action();
        }
    });
}

DispatcherConnectionChain::DispatcherConnectionChain()
    : IsValid(true)
{

}

const Dispatcher* DispatcherConnectionChain::DefaultDispatcher()
{
    static Dispatcher result;
    return &result;
}

void DispatcherConnectionChain::ConnectFrom(const char* cdl, const DispatcherConnectionChain& another)
{
    connectFrom(cdl, another);
    if(another.IsValid) {
        update();
    } else {
        IsValid.SetState(false);
    }
}

void DispatcherConnectionChain::add(const char* cdl, const FDep& dependency)
{
    m_deps.append(dependency);
}

void DispatcherConnectionChain::invalidResult()
{
    IsValid.SetState(false);
}

void DispatcherConnectionChain::validResult()
{
    IsValid.EditSilent() = false;
    IsValid.SetState(true);
}

void DispatcherConnectionChain::connectFrom(const char* cdl, const DispatcherConnectionChain& another)
{
    add(cdl, [&another] {
        return &another.IsValid.OnChanged;
    });
    add(cdl, [&another]() -> const Dispatcher* {
        return another.IsValid ? DefaultDispatcher() : nullptr;
    });
}

void DispatcherConnectionChain::update()
{
    m_depConnections.clear();
    for(const auto& h : m_deps) {
        auto* disp = h();
        if(disp == nullptr) {
            invalidResult();
            return;
        }
        disp->Connect(CDL, [this]{ update(); }).MakeSafe(m_depConnections);
    }
    validResult();
}

StateParametersChainData::StateParametersChainData(StateParameters* params, const FCapture& capture, const FCapture& release)
    : Errors(::make_shared<LocalPropertyErrorsModel>())
    , Processes(::make_shared<LocalPropertyErrorsModel>())
    , m_capture(capture)
    , m_release(release)
    , m_includer([this]{
        m_capture(*this);
    }, [this]{
        m_release(*this);
    })
{
}

void StateParametersChainData::SetCaptureHandler(const FCapture& capture)
{
    m_capture = capture;
    if(m_includer.IsCaptured()) {
        m_release(*this);
        m_capture(*this);
    }
}
