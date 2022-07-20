#ifndef MODELSERRORSCOMPONENTS_H
#define MODELSERRORSCOMPONENTS_H

#include <PropertiesModule/internal.hpp>

struct DescModelsErrorComponentAttachToErrorsContainerParameters
{
    qint64 ErrorFlags;
    bool EnableFilter = true;
    Name ErrorName;
    TranslatedStringPtr Label;
    QtMsgType MessageType = QtWarningMsg;

    DescModelsErrorComponentAttachToErrorsContainerParameters(const Name& errorName, const TranslatedStringPtr& label, qint64 flags);

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

    Dispatcher ErrorsHandled;
    LocalPropertyInt64 ErrorState;
    LocalPropertyInt64 ErrorFilter;
    QHash<Name, qint64> AttachedErrors;

    template<class T2>
    bool HasError(const T2& value, qint64 errorFlags) const
    {
        return value.StateError & ErrorFilter & errorFlags;
    }

    /// deprecated
    template<class T2>
    QVariant WarningIcon(const T2& value, qint64 errorFlags, const struct ModelsIconsContext& iconsContext) const
    {
        return HasError(value, errorFlags) ? QVariant(iconsContext.WarningIcon) : QVariant();
    }

    /// deprecated
    template<class T2>
    QVariant ErrorIcon(const T2& value, qint64 errorFlags, const ModelsIconsContext& iconsContext) const
    {
        return HasError(value, errorFlags) ? QVariant(iconsContext.ErrorIcon) : QVariant();
    }

    /// deprecated
    template<class T2>
    QVariant ErrorIcon(const T2& value, const QVector<std::pair<qint64, QtMsgType>>& sequence, const ModelsIconsContext& iconsContext) const
    {
        for(const auto& [error, type] : sequence) {
            if(HasError(value, error)) {
                switch(type) {
                case QtCriticalMsg: return iconsContext.ErrorIcon;
                case QtWarningMsg: return iconsContext.WarningIcon;
                case QtInfoMsg: return iconsContext.InfoIcon;
                default: break;
                }
                return QVariant();
            }
        }
        return QString();
    }

    template<class T2>
    QVariant ErrorIcon(const T2& value, const QVector<qint64>& sequence, const struct ModelsIconsContext& iconsContext) const
    {
        for(const auto& error : sequence) {
            if(HasError(value, error)) {
                auto foundIt = m_errorTypes.find(error);
                if(foundIt != m_errorTypes.end()) {
                    switch(foundIt.value()) {
                    case QtCriticalMsg: return iconsContext.ErrorIcon;
                    case QtWarningMsg: return iconsContext.WarningIcon;
                    case QtInfoMsg: return iconsContext.InfoIcon;
                    default: break;
                    }
                }
                return QVariant();
            }
        }
        return QString();
    }

    template<class T2>
    QString ErrorString(const T2& value, qint64 errorFlag) const
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

    template<class T2>
    QString ErrorString(const T2& value, const QVector<qint64>& sequence) const
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
        }, { &ErrorState.OnChanged }, parameters.MessageType, visibleProperty);
        if(visibleProperty != nullptr) {
            auto* pProperty = visibleProperty.get();
            visibleProperty->OnChanged.Connect(this, [this,pProperty, errorFlags]{
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

    void RegisterError(qint64 error, const FHandler& checkHandler, const TranslatedStringPtr& errorComment, QtMsgType type = QtCriticalMsg)
    {
        Q_ASSERT(m_errorHandlers.find(error) == m_errorHandlers.end());
        m_errorHandlers.insert({ error, checkHandler });
        if(errorComment != nullptr) {
            m_errorComments.insert(error, errorComment);
        }
        m_errorTypes.insert(error, type);
    }

    void RegisterError(qint64 error, const FPerRowHandler& checkHandler, const TranslatedStringPtr& errorComment, QtMsgType type = QtCriticalMsg)
    {
        Q_ASSERT(m_errorPerRowHandlers.find(error) == m_errorPerRowHandlers.end());
        m_errorPerRowHandlers.insert({ error, checkHandler });
        if(errorComment != nullptr) {
            m_errorComments.insert(error, errorComment);
        }
        m_errorTypes.insert(error, type);
    }

    template<typename... Dispatchers>
    void AddDependencies(const Dispatcher& first, Dispatchers&... dispatchers)
    {
        first.ConnectCombined([this]{ update(); }, dispatchers...);
    }

    void Initialize(const WrapperPtr& wrapper, const std::function<qint64& (T& data)>& flagsGetter =
            [](T& data) -> qint64& {
                return data.StateError;
            }, const std::function<bool (const T& data)>& hasCriticalErrorsHandler = [](const T& data){
                return data.HasCriticalError();
            })
    {
        m_updateHandler = [this, wrapper, flagsGetter, hasCriticalErrorsHandler]{
            guards::LambdaGuard guard([this]{ ErrorsHandled(); });
            if(wrapper->IsEmpty()) {
                ErrorState = 0;
                return;
            }

            auto errorState = 0;
            wrapper->UpdateUi([&errorState, wrapper, this, flagsGetter, hasCriticalErrorsHandler]{
                auto& native = wrapper->EditSilent();
                qint32 startCorrectIndex = 0;
                qint32 index = 0;
                bool foundStart = false;
                for(auto& data : native) {
                    auto& flags = flagsGetter(data);
                    flags = 0;
                    for(const auto& [code, handler] : m_errorPerRowHandlers) {
                        LongFlagsHelpers::ChangeFromBoolean(!handler(data), flags, code);
                    }
                    if(!foundStart && !hasCriticalErrorsHandler(data)) {
                        startCorrectIndex = index;
                        foundStart = true;
                    }
                    ++index;
                }

                auto prev = native.begin() + startCorrectIndex;
                if(prev != native.end()) {
                    errorState |= flagsGetter(*prev);
                }

                for(auto nextIt(native.begin() + startCorrectIndex + 1), endIt(native.end()); nextIt != endIt; ++nextIt) {
                    auto& prevData = *prev;
                    auto& nextData = *nextIt;
                    auto& flags = flagsGetter(nextData);
                    for(const auto& [code, handler] : m_errorHandlers) {
                        LongFlagsHelpers::ChangeFromBoolean(!handler(nextData, prevData), flags, code);
                    }
                    errorState |= flags;
                    if(!hasCriticalErrorsHandler(nextData)) {
                        prev = nextIt;
                    }
                }
            });

            ErrorState = errorState;

        };

        onInitialize(wrapper);
    }

    void InitializePerRowOnly(const WrapperPtr& wrapper, const std::function<qint64& (T& data)>& flagsGetter =
            [](T& data) -> qint64& {
                return data.StateError;
            }, bool updateOnWrapperChanged = true)
    {
        m_updateHandler = [this, wrapper, flagsGetter]{
            guards::LambdaGuard guard([this]{ ErrorsHandled(); });
            if(wrapper->IsEmpty()) {
                ErrorState = 0;
                return;
            }

            auto errorState = 0;
            wrapper->UpdateUi([&errorState, wrapper, this, flagsGetter]{
                auto& native = wrapper->EditSilent();
                for(auto& data : native) {
                    auto& flags = flagsGetter(data);
                    for(const auto& [code, handler] : m_errorPerRowHandlers) {
                        LongFlagsHelpers::ChangeFromBoolean(!handler(data), flags, code);
                    }
                    errorState |= flags;
                }
            });

            ErrorState = errorState;
        };

        onInitialize(wrapper, updateOnWrapperChanged);
    }

private:
    void update()
    {
        m_updater.Call(CONNECTION_DEBUG_LOCATION, [this]{
            m_updateHandler();
        });
    }

    void onInitialize(const WrapperPtr& wrapper, bool updateOnWrapperChanged = true)
    {
        ErrorFilter.OnChanged.Connect(this, [wrapper]{ wrapper->UpdateUi([]{}); });

        if(updateOnWrapperChanged) {
            wrapper->OnChanged.Connect(this, [this]{
                update();
            }).MakeSafe(m_connection);
        }
        update();
    }

private:
    DispatcherConnectionsSafe m_connection;
    std::map<qint64, FHandler> m_errorHandlers;
    std::map<qint64, FPerRowHandler> m_errorPerRowHandlers;
    QHash<qint64, TranslatedStringPtr> m_errorComments;
    QHash<qint64, QtMsgType> m_errorTypes;
    DelayedCallObject m_updater;
    FAction m_updateHandler;
    QHash<qint64, DescModelsErrorComponentAttachToErrorsContainerParameters> m_errors;
};

#endif // MODELSERRORSCOMPONENTS_H
