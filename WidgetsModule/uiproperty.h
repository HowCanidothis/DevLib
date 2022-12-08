#ifndef UIPROPERTY_H
#define UIPROPERTY_H

#include <PropertiesModule/internal.hpp>>
#include <UnitsModule/internal.hpp>

template <typename T>
class UIPropertyWrapper : public T
{
    using Super = T;
public:
    using Super::Super;

    Name Id;
    T& Base() { return *this; }
    const T& Base() const { return *this; }

    const Measurement* Unit;
    TranslatedStringPtr Label;
    TranslatedStringPtr ErrorMessage;

    UIPropertyWrapper& RegisterUI(const Name& id, const TranslatedStringPtr& label, const Measurement* measurement = nullptr){
        Q_ASSERT(Id.IsNull());
        Id = id;
        Label = label;
        Unit = measurement;
        return *this;
    }

    using ErrorPropertyHandler = std::function<const LocalPropertyBool&(const Super&)>;
    using ErrorMessageHandler = std::function<QString(const QString&)>;

    DispatcherConnections RegisterErrorIn(LocalPropertyErrorsContainer& errors, const ErrorPropertyHandler& errorHandler, QtMsgType severity = QtCriticalMsg){
        Q_ASSERT(!Id.IsNull());
        if(ErrorMessage == nullptr) {
            Q_ASSERT(Label != nullptr);
            ErrorMessage = TRS(QObject::tr("%1 is not set").arg(Label->Native()), this);
        }
        return errors.RegisterError(Id, ErrorMessage, errorHandler(*this), true, severity);
    }
    DispatcherConnections RegisterErrorIn(LocalPropertyErrorsContainer& errors, QtMsgType severity = QtCriticalMsg);
};

template<>
inline DispatcherConnections UIPropertyWrapper<StateParameterProperty<LocalPropertyDoubleOptional>>::RegisterErrorIn(LocalPropertyErrorsContainer& errors, QtMsgType severity){
    Q_ASSERT(!Id.IsNull());
    return RegisterErrorIn(errors, [](const StateParameterProperty<LocalPropertyDoubleOptional>& p) -> const LocalPropertyBool& { return p.InputValue.IsValid; }, severity);
}
template<>
inline DispatcherConnections UIPropertyWrapper<LocalPropertyDoubleOptional>::RegisterErrorIn(LocalPropertyErrorsContainer& errors, QtMsgType severity){
    Q_ASSERT(!Id.IsNull());
    return RegisterErrorIn(errors, [](const LocalPropertyDoubleOptional& p) -> const LocalPropertyBool& { return p.IsValid; }, severity);
}


#endif // UIPROPERTY_H
