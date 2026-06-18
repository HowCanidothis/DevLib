#include "localpropertyerrorscontainer.h"

#include "translatormanager.h"

void LocalPropertyErrorsModel::DisconnectFromExternalModels()
{
    m_containerConnections.clear();
    QVector<std::pair<Name, const LocalPropertyErrorsModel*>> externErrors;
    for(auto it(Super::begin()), e(Super::end()); it != e; ++it) {
        for(const auto* c : it.value()) {
            if(c != this) {
                externErrors.append(std::make_pair(it.key(), c));
            }
        }
    }
    for(const auto& c : externErrors) {
        Remove(c.first, c.second);
    }
}

LocalPropertyErrorsModel& LocalPropertyErrorsModel::ConnectFromModels(const char* cdl, const QVector<const LocalPropertyErrorsModel*>& containers, const FFilterFunc& filterFunc)
{
    if(!m_containerConnections.isEmpty()) {
        DisconnectFromExternalModels();
    }
    for(const LocalPropertyErrorsModel* errorContainer : containers) {
        auto add = [this, filterFunc, errorContainer](const Name& id){
            if(filterFunc(id)) {
                Add(id, errorContainer);
            }
        };
        auto remove = [this, errorContainer, filterFunc](const Name& id) {
            if(filterFunc(id)) {
                Remove(id, errorContainer);
            }
        };
        errorContainer->OnErrorAdded.Connect(cdl, add).MakeSafe(m_containerConnections);
        errorContainer->OnErrorRemoved.Connect(cdl, remove).MakeSafe(m_containerConnections);
        for(const auto& id : *errorContainer) {
            add(id);
        }
    }
    return *this;
}

LocalPropertyErrorsModel& LocalPropertyErrorsModel::RegisterBool(const char* cdl, const Name& errorId, const LocalPropertyBool& property, bool inverted)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_registeredErrors.contains(errorId));
    m_registeredErrors.insert(errorId);
#endif
    property.ConnectAndCall(cdl, [this, errorId, inverted](bool value){
        if(value ^ inverted) {
            Add(errorId);
        } else {
            Remove(errorId);
        }
    }).MakeSafe(m_connections);
    return *this;
}

LocalPropertyErrorsModel& LocalPropertyErrorsModel::Register(const char* cdl, const Name& errorId, const std::function<bool ()>& validator, const QVector<Dispatcher*>& dispatchers)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_registeredErrors.contains(errorId));
    m_registeredErrors.insert(errorId);
#endif
    auto update = [this, validator, errorId]{
        if(!validator()) {
            Add(errorId);
        } else {
            Remove(errorId);
        }
    };

    for(auto* dispatcher : dispatchers) {
        dispatcher->Connect(cdl, update).MakeSafe(m_connections);
    }

    update();
    return *this;
}

void LocalPropertyErrorsModel::Set(const QSet<Name>& ids)
{
    Clear();
    QHash<Name, QSet<const LocalPropertyErrorsModel*>> r;
    for(const auto& id : ids) {
        r.insert(id, {this});
    }
    Super::operator=(r);
    for(const auto& id : ids) {
        OnErrorAdded(id);
    }
}

void LocalPropertyErrorsModel::Clear()
{
#ifdef QT_DEBUG
    m_registeredErrors.clear();
#endif
    m_containerConnections.clear();
    for(const auto& id : *this) {
        OnErrorRemoved(id);
    }
    Super::Clear();
}

void LocalPropertyErrorsModel::Remove(const LocalPropertyErrorsModel* errors)
{
    QVector<Name> errorsToRemove;
    for(auto it(Super::begin()), e(Super::end()); it != e; ++it) {
        if(it.value().contains(errors)) {
            errorsToRemove.append(it.key());
        }
    }
    for(const auto& error: errorsToRemove) {
        Remove(error, errors);
    }
}

LocalPropertyErrorsModel::LocalPropertyErrorsModel()
    : IsValid(true)
{
    Connect(CDL, [this](const auto& errors){
        IsValid.SetState(errors.isEmpty());
    });
    adapters::ResetThread(OnChanged);
}

void LocalPropertyErrorsModel::Add(const Name& errorName, const LocalPropertyErrorsModel* source)
{
    auto& s = Super::EditSilent();
    auto foundIt = s.find(errorName);
    if(foundIt != s.end()) {
        foundIt.value().insert(source);
        return;
    }
    if(Super::Insert(errorName, {source})) {
        OnErrorAdded(errorName);
    }
    return;
}

void LocalPropertyErrorsModel::Remove(const Name& errorName, const LocalPropertyErrorsModel* source)
{
    auto& s = Super::EditSilent();
    auto foundIt = s.find(errorName);
    if(foundIt != s.end()) {
        if(foundIt.value().remove(source)) {
            if(foundIt.value().isEmpty()) {
                Super::Remove(errorName);
                OnErrorRemoved(errorName);
            }
        }
    }
}

bool LocalPropertyErrorsModel::Has(const Name& errorName) const
{
    return Super::IsContains(errorName);
}

namespace internal {
QHash<Name, LocalPropertyErrorsViewModelDescription> GlobalErrorDescriptions;
}

LocalPropertyErrorsViewModel::LocalPropertyErrorsViewModel()
    : IsValid(true)
    , HasErrorsOrWarnings(false)
    , OnErrorsLabelsChanged(500)
    , m_model(new LocalPropertyErrorsModel())
    , m_internalModel(true)
    , m_useGlobalDescriptions(true)
{
    init();
}

LocalPropertyErrorsViewModel::LocalPropertyErrorsViewModel(LocalPropertyErrorsModel* model)
    : IsValid(true)
    , HasErrorsOrWarnings(false)
    , OnErrorsLabelsChanged(500)
    , m_model(model)
    , m_internalModel(false)
    , m_useGlobalDescriptions(true)
{
    init();
}

void LocalPropertyErrorsViewModel::SetUseGlobalDescriptions()
{
    m_getDescDelegate = [this](const Name& id) -> const LocalPropertyErrorsViewModelDescription* {
        auto foundIt = m_viewModel.constFind(id);
        if(foundIt == m_viewModel.cend()) {
            auto errIt = internal::GlobalErrorDescriptions.constFind(id);
            if(errIt == internal::GlobalErrorDescriptions.cend()) {
                return nullptr;
            }
            return &errIt.value();
        }
        return &foundIt.value();
    };
}

LocalPropertyErrorsViewModel::~LocalPropertyErrorsViewModel()
{
    if(m_internalModel) {
        delete m_model;
    }
}

void LocalPropertyErrorsViewModel::AddAndRegisterError(const Name& errorName, const RegisterParams& params)
{
    Q_ASSERT(!m_model->Has(errorName));
    m_viewModel.insert(errorName, params.Value);
    AddError(errorName);
}

void LocalPropertyErrorsViewModel::RemoveAndUnregisterError(const Name& errorName)
{
    if(HasError(errorName)) {
        RemoveError(errorName);
    }
    m_viewModel.remove(errorName);
    Q_ASSERT(!m_model->Has(errorName));
}

LocalPropertyErrorsViewModelDescription* LocalPropertyErrorsViewModel::EditDescription(const Name& id)
{
    Q_ASSERT(!m_useGlobalDescriptions);
    return const_cast<LocalPropertyErrorsViewModelDescription*>(find(id));
}

const LocalPropertyErrorsViewModelDescription& LocalPropertyErrorsViewModel::GetDescription(const Name& id) const
{
    static const LocalPropertyErrorsViewModelDescription defaultDesc(TRS("Unregistered Error"));
    auto* found = find(id);
    if(found == nullptr) {
        return defaultDesc;
    }
    return *found;
}

LocalPropertyErrorsViewModel& LocalPropertyErrorsViewModel::Register(const Name& errorId, const RegisterParams& params)
{
    Q_ASSERT(!m_viewModel.contains(errorId));
    m_viewModel.insert(errorId, params.Value);
    return *this;
}

LocalPropertyErrorsViewModel& LocalPropertyErrorsViewModel::ConnectFromModels(const char* cdl, const QVector<const LocalPropertyErrorsModel*>& containers, const LocalPropertyErrorsModel::FFilterFunc& filter)
{
    GetModel()->ConnectFromModels(cdl, containers, [this, filter](const Name& id) {
        return find(id) && filter(id);
    });
    return *this;
}

QString LocalPropertyErrorsViewModel::ToString() const
{
    QString resultText;
    for(const auto& id : *this) {
        if(!find(id)) {
            resultText += id.AsString();
        } else {
            resultText += GetDescription(id).Text->Native() + "\n";
        }
    }
    return resultText;
}

QString LocalPropertyErrorsViewModel::ToErrorString(const QString& separator) const
{
    QString resultText;
    for(const auto& id : *this) {
        if(!find(id)) {
            resultText += id.AsString() + separator;
        } else {
            const auto& desc = GetDescription(id);
            if(desc.IsError()){
                resultText += desc.Text->Native() + separator;
            }
        }
    }
    return resultText;
}

QStringList LocalPropertyErrorsViewModel::ToStringList() const
{
    QStringList result;
    for(const auto& id : *this) {
        if(!find(id)) {
            result += id.AsString();
        } else {
            const auto& desc = GetDescription(id);
            if(desc.IsError()){
                result += desc.Text->Native();
            }
        }
    }
    return result;
}

void LocalPropertyErrorsViewModel::init()
{
    m_getDescDelegate = [this](const Name& id) -> const LocalPropertyErrorsViewModelDescription* {
        auto foundIt = m_viewModel.constFind(id);
        if(foundIt == m_viewModel.cend()) {
            return nullptr;
        }
        return &foundIt.value();
    };
    auto labelsConnections = DispatcherConnectionsSafeCreate();
    auto connection = m_model->OnChanged.Connect(CDL, m_updateState.Wrap(CDL, [this, labelsConnections]{
        QVector<Name> registeredIds;
        for(const auto& value : *this) {
            if(find(value)) {
                registeredIds.append(value);
            }
        }
        HasErrorsOrWarnings = !registeredIds.isEmpty();
        for(const auto& value : registeredIds) {
            OnErrorsLabelsChanged.ConnectFrom(CDL, GetDescription(value).Text->OnChanged).MakeSafe(*labelsConnections);
        }
        for(const auto& value : registeredIds) {
            if(GetDescription(value).IsError()) {
                IsValid.SetState(false);
                return;
            }
        }
        IsValid.SetState(true);
    }, [this, labelsConnections]{
        labelsConnections->clear();
        IsValid.SetState(false);
        HasErrorsOrWarnings = !m_model->IsEmpty();
    }));
    if(!m_internalModel) {
        connection.MakeSafe(m_connections);
    }

//    TranslatorManager::GetInstance().OnLanguageChanged.Connect(CDL, [this]{
//        OnErrorsLabelsChanged();
    //    }).MakeSafe(m_connections);
}

const LocalPropertyErrorsViewModelDescription* LocalPropertyErrorsViewModel::find(const Name& id) const
{
    return m_getDescDelegate(id);
}
