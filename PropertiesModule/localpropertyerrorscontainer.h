#ifndef LOCALPROPERTYERRORSCONTAINER_H
#define LOCALPROPERTYERRORSCONTAINER_H

#include "localproperty.h"
#include "localpropertydeclarations.h"

class LocalPropertyErrorsModel : public LocalPropertyHash<Name, QSet<const LocalPropertyErrorsModel*>>
{
using Super = LocalPropertyHash<Name, QSet<const LocalPropertyErrorsModel*>>;
public:
    LocalPropertyErrorsModel();

    void Add(const Name& errorName) { Add(errorName, this); }
    void Remove(const Name& errorName) { Remove(errorName, this); }
    void Add(const Name& errorName, const LocalPropertyErrorsModel* errors);
    void Remove(const Name& errorName, const LocalPropertyErrorsModel* errors);
    void Remove(const LocalPropertyErrorsModel* errors);
    bool Has(const Name& errorName) const;
    template<class Container>
    bool HasAny(const Container& errorName) const
    {
        for(const auto& id : errorName) {
            if(Super::IsContains(id)) {
                return true;
            }
        }
        return false;
    }

    void Set(const QSet<Name>& id);
    void Clear();
    void DisconnectFromExternalModels();

    template<class T>
    LocalPropertyErrorsModel& RegisterOptional(const char* cdl, const Name& errorId, const LocalPropertyOptional<T>& property)
    {
        return RegisterBool(cdl, errorId, property.IsValid, true);
    }
    template<typename T>
    LocalPropertyErrorsModel& RegisterOptional(const char* cdl, const Name& errorId, const LocalPropertyBool& include, const LocalPropertyOptional<T>& property){
        return Register(cdl, errorId, [](bool include, bool isValid){ return !include || isValid; }, include, property.IsValid);
    }
    LocalPropertyErrorsModel& RegisterBool(const char* cdl, const Name& errorId, const LocalPropertyBool& property, bool inverted = false);
    LocalPropertyErrorsModel& Register(const char* cdl, const Name& errorId, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers);

    template<typename Function, typename ... Args>
    LocalPropertyErrorsModel& Register(const char* cdl, const Name& errorId, const Function& validator, const Args& ... args) {
#ifdef QT_DEBUG
        Q_ASSERT(!m_registeredErrors.contains(errorId));
        m_registeredErrors.insert(errorId);
#endif
        auto update = [this, validator, errorId, &args...]{
            if(!validator(args.Native()...)) {
                Add(errorId);
            } else {
                Remove(errorId);
            }
        };

        adapters::Combine([&](const auto& property){
            property.ConnectAction(cdl, update).MakeSafe(m_connections);
        }, args...);
        update();
        return *this;
    }

    DispatcherConnection ConnectFrom(const LocalPropertyHash<Name,qint32>& another) const = delete;

    template<class ... ChildContainers>
    LocalPropertyErrorsModel& ConnectFromModels(const char* cdl, const LocalPropertyErrorsModel& errorsModel, const ChildContainers&... errors)
    {
        QVector<const LocalPropertyErrorsModel*> containers;
        adapters::Combine([&](const auto& c){
            containers.append(&c);
        }, errorsModel, errors...);
        return ConnectFromModels(cdl, containers);
    }

    using FFilterFunc = std::function<bool (const Name&)>;
    template<class ... ChildContainers>
    LocalPropertyErrorsModel& ConnectFromModels(const char* cdl, const FFilterFunc& filterFunc, const LocalPropertyErrorsModel& errorsModel, const ChildContainers&... errors)
    {
        QVector<const LocalPropertyErrorsModel*> containers;
        adapters::Combine([&](const auto& c){
            containers.append(&c);
        }, errorsModel, errors...);
        return ConnectFromModels(cdl, containers, filterFunc);
    }

    LocalPropertyErrorsModel& ConnectFromModels(const char* cdl, const QVector<const LocalPropertyErrorsModel*>& containers, const FFilterFunc& filterFunc = [](const Name&) { return true; });

    Super::value_type::key_iterator begin() const { return Native().keyBegin(); }
    Super::value_type::key_iterator end() const { return Native().keyEnd(); }

    CommonDispatcher<const Name&> OnErrorAdded;
    CommonDispatcher<const Name&> OnErrorRemoved;
    StateProperty IsValid;

private:
#ifdef QT_DEBUG
    QSet<Name> m_registeredErrors;
#endif
    DispatcherConnectionsSafe m_containerConnections;
    DispatcherConnectionsSafe m_connections;
};

struct LocalPropertyErrorsViewModelDescription
{
    LocalPropertyErrorsViewModelDescription(const TranslatedStringPtr& text = nullptr)
        : Text(text)
    {}

    TranslatedStringPtr Text;
    QtMsgType Severity = QtMsgType::QtCriticalMsg;
    SharedPointer<LocalPropertyBool> Visible; // is used for table error filter
    QVariant Metadata;
    FAction ActivationHandler;

    bool IsError() const { return Severity == QtCriticalMsg || Severity == QtFatalMsg; }
};

#define LPEVMPARAMS \
    LocalPropertyErrorsViewModel::RegisterParams

class LocalPropertyErrorsViewModel
{
public:
    struct RegisterParams
    {
        LocalPropertyErrorsViewModelDescription Value;
        RegisterParams(const TranslatedStringPtr& text)
            : Value(text)
        {}

        RegisterParams& SetSeverity(QtMsgType severity)
        {
            Value.Severity = severity;
            return *this;
        }

        RegisterParams& SetMetadata(const QVariant& metadata)
        {
            Value.Metadata = metadata;
            return *this;
        }

        RegisterParams& SetVisible(const SP<LocalPropertyBool>& visible)
        {
            Value.Visible = visible;
            return *this;
        }
        RegisterParams& SetActivationHandler(const FAction& handler)
        {
            Value.ActivationHandler = handler;
            return *this;
        }
    };

    LocalPropertyErrorsViewModel();
    LocalPropertyErrorsViewModel(LocalPropertyErrorsModel* model);
    ~LocalPropertyErrorsViewModel();

    void SetUseGlobalDescriptions();

    void AddError(const Name& errorName) { m_model->Add(errorName); }
    void RemoveError(const Name& errorName) { m_model->Remove(errorName); }

    void AddAndRegisterError(const Name& errorName, const RegisterParams& params);
    void RemoveAndUnregisterError(const Name& errorName);

    bool HasError(const Name& errorName) const { return m_model->Has(errorName); }
    LocalPropertyErrorsViewModelDescription* EditDescription(const Name& id);
    const LocalPropertyErrorsViewModelDescription& GetDescription(const Name& id) const;
    const QHash<Name, LocalPropertyErrorsViewModelDescription>& GetDescriptions() const { return m_viewModel; }

    const LocalPropertyErrorsModel* GetModel() const { return m_model; }
    LocalPropertyErrorsModel* GetModel() { return m_model; }

    LocalPropertyErrorsViewModel& Register(const Name& errorId, const RegisterParams& params);

    template<class T>
    LocalPropertyErrorsViewModel& RegisterOptional(const char* cdl, const Name& errorId, const RegisterParams& params, const LocalPropertyOptional<T>& property)
    {
        m_model->RegisterOptional(cdl, errorId, property);
        Register(errorId, params);
        return *this;
    }
    template<typename T>
    LocalPropertyErrorsViewModel& RegisterOptional(const char* cdl, const Name& errorId, const RegisterParams& params, const LocalPropertyBool& include, const LocalPropertyOptional<T>& property){
        m_model->RegisterOptional(cdl, errorId, property);
        Register(errorId, params);
        return *this;
    }
    LocalPropertyErrorsViewModel& RegisterBool(const char* cdl, const Name& errorId, const RegisterParams& params, const LocalPropertyBool& property, bool inverted = false)
    {
        m_model->RegisterBool(cdl, errorId, property, inverted);
        Register(errorId, params);
        return *this;
    }

    LocalPropertyErrorsViewModel& Register(const char* cdl, const Name& errorId, const RegisterParams& params, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers)
    {
        m_model->Register(cdl, errorId, validator, dispatchers);
        Register(errorId, params);
        return *this;
    }

    template<typename Function, typename ... Args>
    LocalPropertyErrorsViewModel& Register(const char* cdl, const Name& errorId, const RegisterParams& params, const Function& validator, const Args& ... args) {
        m_model->Register(cdl, errorId, validator, args...);
        Register(errorId, params);
        return *this;
    }

    template<class ... ChildContainers>
    LocalPropertyErrorsViewModel& ConnectFromModels(const char* cdl, const LocalPropertyErrorsModel& errorsModel, const ChildContainers&... errors)
    {
        QVector<const LocalPropertyErrorsModel*> containers;
        adapters::Combine([&](const auto& c){
            containers.append(&c);
        }, errorsModel, errors...);
        return ConnectFromModels(cdl, containers);
    }

    template<class ... ChildContainers>
    LocalPropertyErrorsViewModel& ConnectFromModels(const char* cdl, const LocalPropertyErrorsModel::FFilterFunc& filter, const LocalPropertyErrorsModel& errorsModel, const ChildContainers&... errors)
    {
        QVector<const LocalPropertyErrorsModel*> containers;
        adapters::Combine([&](const auto& c){
            containers.append(&c);
        }, errorsModel, errors...);
        return ConnectFromModels(cdl, containers, filter);
    }

    LocalPropertyErrorsViewModel& ConnectFromModels(const char* cdl, const QVector<const LocalPropertyErrorsModel*>& containers, const LocalPropertyErrorsModel::FFilterFunc& filter = [](const Name&) { return true; });

    LocalPropertyErrorsModel::value_type::key_iterator begin() const { return m_model->begin(); }
    LocalPropertyErrorsModel::value_type::key_iterator end() const { return m_model->end(); }

    QString ToString() const;
    QString ToErrorString(const QString& separator = "\n") const;
    QStringList ToStringList() const;

    StateProperty IsValid; // If has no errors then IsValid == true
    LocalPropertyBool HasErrorsOrWarnings;
    DispatchersCommutator OnErrorsLabelsChanged;

private:
    void init();
    const LocalPropertyErrorsViewModelDescription* find(const Name& id) const;

private:
    QHash<Name, LocalPropertyErrorsViewModelDescription> m_viewModel;
    LocalPropertyErrorsModel* m_model;
    bool m_internalModel:1;
    bool m_useGlobalDescriptions:1;
    DelayedCallObject m_updateState;
    DispatcherConnectionsSafe m_connections;
    std::function<const LocalPropertyErrorsViewModelDescription* (const Name& id)> m_getDescDelegate;
};

namespace internal {
extern QHash<Name, LocalPropertyErrorsViewModelDescription> GlobalErrorDescriptions;
}

struct etr
{
    Q_DECLARE_TR_FUNCTIONS(etr);
};

#define DECLARE_GLOBAL_ERROR(id) \
namespace Error { \
    DECLARE_GLOBAL_NAME(id); \
} \
namespace ErrorDesc { \
    DECLARE_GLOBAL(LocalPropertyErrorsViewModel::RegisterParams, id); \
}

#define IMPLEMENT_GLOBAL_ERROR(id, params) \
namespace Error { \
    IMPLEMENT_GLOBAL_NAME_1(id); \
} \
namespace ErrorDesc { \
    const LocalPropertyErrorsViewModel::RegisterParams id = []{ \
        LocalPropertyErrorsViewModel::RegisterParams result(params); \
        internal::GlobalErrorDescriptions.insert(Error::id,result.Value); \
        return result; \
    }(); \
}


#define COMBINE_GLOBAL_ERROR(id) \
    Error::id, ErrorDesc::id

#define OVERRIDE_GLOBAL_ERROR(id) \
    Error::id, LPEVMPARAMS(ErrorDesc::id)

#define COMBINE_GLOBAL_ERROR_2(ns, id) \
    ns::Error::id, ns::ErrorDesc::id

#define OVERRIDE_GLOBAL_ERROR_2(ns, id) \
    ns::Error::id, LPEVMPARAMS(ns::ErrorDesc::id)

DECLARE_GLOBAL_ERROR(SP_IncompleteData)

#endif // LOCALPROPERTYERRORSCONTAINER_H
