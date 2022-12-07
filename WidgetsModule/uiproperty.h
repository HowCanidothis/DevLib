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
    LocalPropertyErrorsContainer* Errros;

    UIPropertyWrapper& RegisterUI(const Name& id, const TranslatedStringPtr& label, const Measurement* measurement = nullptr){
        Q_ASSERT(Id.IsNull());
        Id = id;
        Label = label;
        Unit = measurement;
        return *this;
    }

    using ErrorPropertyHandler = std::function<const LocalPropertyBool&(const Super&)>;
    using ErrorMessageHandler = std::function<QString(const QString&)>;

    DispatcherConnections RegisterErrorIn(LocalPropertyErrorsContainer& errors, const ErrorPropertyHandler& errorHandler, const ErrorMessageHandler& errorMessage = [](const QString& v){ return QObject::tr("%1 is not set").arg(v);}){
        Q_ASSERT(!Id.IsNull());
        return errors.RegisterError(Id, ::make_shared<TranslatedString>([this, errorMessage]{ return errorMessage(Label->Native()); }), errorHandler(*this));
    }
    DispatcherConnections RegisterErrorIn(LocalPropertyErrorsContainer& errors, const ErrorMessageHandler& errorMessage = [](const QString& v){ return QObject::tr("%1 is not set").arg(v);});
};

template<>
inline DispatcherConnections UIPropertyWrapper<StateParameterProperty<LocalPropertyDoubleOptional>>::RegisterErrorIn(LocalPropertyErrorsContainer& errors, const UIPropertyWrapper::ErrorMessageHandler& errorMessage){
    Q_ASSERT(!Id.IsNull());
    return RegisterErrorIn(errors, [](const StateParameterProperty<LocalPropertyDoubleOptional>& p) -> const LocalPropertyBool& { return p.InputValue.IsValid; }, errorMessage);
}
template<>
inline DispatcherConnections UIPropertyWrapper<LocalPropertyDoubleOptional>::RegisterErrorIn(LocalPropertyErrorsContainer& errors, const UIPropertyWrapper::ErrorMessageHandler& errorMessage){
    Q_ASSERT(!Id.IsNull());
    return RegisterErrorIn(errors, [](const LocalPropertyDoubleOptional& p) -> const LocalPropertyBool& { return p.IsValid; }, errorMessage);
}


#endif // UIPROPERTY_H
