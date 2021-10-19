#ifndef MODELSERRORSCOMPONENTS_H
#define MODELSERRORSCOMPONENTS_H

#include <PropertiesModule/internal.hpp>

struct DescModelsErrorComponentAttachToErrorsContainerParameters
{
    qint32 ErrorFlags;
    bool EnableFilter = true;
    Name ErrorName;
    TranslatedStringPtr Label;
    QtMsgType MessageType = QtWarningMsg;

    DescModelsErrorComponentAttachToErrorsContainerParameters(const Name& errorName, const TranslatedStringPtr& label, qint32 flags);

    DescModelsErrorComponentAttachToErrorsContainerParameters& SetFilterEnabled(bool enabled) { EnableFilter = enabled; return *this; }
    DescModelsErrorComponentAttachToErrorsContainerParameters& SetSeverity(QtMsgType messageType) { MessageType = messageType; return *this; }
};

template <class Wrapper>
class ModelsErrorComponent {
public:
    using WrapperPtr = SharedPointer<Wrapper>;
    using T = typename Wrapper::value_type;
    using FHandler = std::function<bool(const T& current, const T& prev)>;
    using FPerRowHandler = std::function<bool(const T& current)>;

    ModelsErrorComponent()
        : ErrorFilter(0xffffffff)
        , m_updater(1000)
    {}

    LocalPropertyInt ErrorState;
    LocalPropertyInt ErrorFilter;
    QHash<Name, qint32> AttachedErrors;

    bool HasError(const T& value, qint32 errorFlags) const
    {
        return value.StateError & ErrorFilter & errorFlags;
    }

    QVariant WarningIcon(const T& value, qint32 errorFlags, const struct ModelsIconsContext& iconsContext) const
    {
        return HasError(value, errorFlags) ? QVariant(iconsContext.WarningIcon) : QVariant();
    }

    QVariant ErrorIcon(const T& value, qint32 errorFlags, const ModelsIconsContext& iconsContext) const
    {
        return HasError(value, errorFlags) ? QVariant(iconsContext.ErrorIcon) : QVariant();
    }

    QString ErrorString(const T& value, qint32 errorFlag) const
    {
        if(!HasError(value, errorFlag)) {
            return QString();
        }
        auto foundIt = m_errorComments.find(errorFlag);
        if(foundIt != m_errorComments.end()) {
            return foundIt.value()->Native();
        }
        return QString();
    }

    QString ErrorString(const T& value, const QVector<qint32>& sequence) const
    {
        for(auto error : sequence) {
            auto errorString = ErrorString(value, error);
            if(!errorString.isEmpty()) {
                return errorString;
            }
        }
        return QString();
    }

    void AttachToErrorsContainer(LocalPropertyErrorsContainer* container, const DescModelsErrorComponentAttachToErrorsContainerParameters& parameters)
    {
        Q_ASSERT(!AttachedErrors.contains(parameters.ErrorName) && !m_errors.contains(parameters.ErrorFlags));
        auto errorFlags = parameters.ErrorFlags;
        Q_ASSERT(!parameters.ErrorName.IsNull() && parameters.Label != nullptr && errorFlags != 0);
        SharedPointer<LocalPropertyBool> visibleProperty = parameters.EnableFilter ? ::make_shared<LocalPropertyBool>(true) : nullptr;
        container->RegisterError(parameters.ErrorName, parameters.Label, [this, errorFlags]{
            return !(ErrorState.Native() & errorFlags);
        }, { &ErrorState.OnChange }, parameters.MessageType, visibleProperty);
        if(visibleProperty != nullptr) {
            auto* pProperty = visibleProperty.get();
            visibleProperty->OnChange.Connect(this, [this,pProperty, errorFlags]{
                auto errorFilter = ErrorFilter.Native();
                if(*pProperty) {
                    errorFilter |= errorFlags;
                } else {
                    errorFilter &= ~errorFlags;
                }
                ErrorFilter = errorFilter;
            }).MakeSafe(m_connection);
        }
        AttachedErrors.insert(parameters.ErrorName, parameters.ErrorFlags);
    }

    void RegisterError(int error, const FHandler& checkHandler, const TranslatedStringPtr& errorComment)
    {
        Q_ASSERT(m_errorHandlers.find(error) == m_errorHandlers.end());
        m_errorHandlers.insert({ error, checkHandler });
        if(errorComment != nullptr) {
            m_errorComments.insert(error, errorComment);
        }
    }

    void RegisterError(int error, const FPerRowHandler& checkHandler, const TranslatedStringPtr& errorComment)
    {
        Q_ASSERT(m_errorPerRowHandlers.find(error) == m_errorPerRowHandlers.end());
        m_errorPerRowHandlers.insert({ error, checkHandler });
        if(errorComment != nullptr) {
            m_errorComments.insert(error, errorComment);
        }
    }

    void AddDependencies(const QVector<Dispatcher*>& dispatchers)
    {
        for(auto depend : dispatchers){
            depend->Connect(this, [this]{
                update();
            }).MakeSafe(m_connection);
        }
    }

    void Initialize(const WrapperPtr& wrapper) {
        m_updateHandler = [this, wrapper]{
            if(wrapper->IsEmpty()) {
                ErrorState = 0;
                return;
            }

            auto errorState = 0;
            wrapper->UpdateUi([&errorState, wrapper, this]{
                auto& native = wrapper->EditSilent();
                auto prev = native.begin();
                for(const auto& [code, handler] : m_errorPerRowHandlers) {
                    auto& data = *prev;
                    auto& flags = data.StateError;
                    flags.ChangeFromBoolean(code, !handler(data));
                    errorState |= flags;
                }
                if((*prev).HasCriticalError()) {
                    return;
                }
                for(auto nextIt(native.begin() + 1), endIt(native.end()); nextIt != endIt; ++nextIt) {
                    auto& prevData = *prev;
                    auto& nextData = *nextIt;
                    auto& flags = nextData.StateError;
                    for(const auto& [code, handler] : m_errorPerRowHandlers) {
                        flags.ChangeFromBoolean(code, !handler(nextData));
                    }
                    for(const auto& [code, handler] : m_errorHandlers) {
                        flags.ChangeFromBoolean(code, !handler(nextData, prevData));
                    }
                    errorState |= flags;
                    if(!nextData.HasCriticalError()) {
                        prev = nextIt;
                    }
                }
            });

            ErrorState = errorState;
        };

        ErrorFilter.OnChange.Connect(this, [wrapper]{ wrapper->Update([]{}); });

        wrapper->OnRowsChanged.Connect(this, [this](int, int, const QSet<int>&){
            update();
        }).MakeSafe(m_connection);
        wrapper->OnRowsInserted.Connect(this, [this](int, int){
            update();
        }).MakeSafe(m_connection);
        wrapper->OnReseted.Connect(this, [this]{
            update();
        }).MakeSafe(m_connection);
        update();
    }

private:
    void update()
    {
        m_updater.Call([this]{
            m_updateHandler();
        });
    }

private:
    DispatcherConnectionsSafe m_connection;
    std::map<qint32, FHandler> m_errorHandlers;
    std::map<qint32, FPerRowHandler> m_errorPerRowHandlers;
    QHash<qint32, TranslatedStringPtr> m_errorComments;
    DelayedCallObject m_updater;
    FAction m_updateHandler;
    QHash<qint32, DescModelsErrorComponentAttachToErrorsContainerParameters> m_errors;
};

#endif // MODELSERRORSCOMPONENTS_H
