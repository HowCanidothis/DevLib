#include "propertypromise.h"

PropertyPromiseBase::PropertyPromiseBase(const Name& name, qint32 contextIndex)
    : _getter([name, contextIndex, this]{
        auto context = PropertiesSystem::context(contextIndex);
        auto find = context.find(name);
        Q_ASSERT_X(find != context.end(), "PropertiesSystem::getValue", name.AsString().toLatin1().constData());
        auto property = find.value();
        _getter = [property]{ return property; };
        return property;
    })
    , _isValid([name, contextIndex, this]{
        auto context = PropertiesSystem::context(contextIndex);
        if(context.contains(name)) {
            _isValid = []{ return true; };
        } else {
            return false;
        }
        return true;
    })
    , _setter([name, contextIndex, this](const QVariant& value){
        auto context = PropertiesSystem::context(contextIndex);
        auto find = context.find(name);
        Q_ASSERT_X(find != context.end(), "PropertiesSystem::setValue", name.AsString().toLatin1().constData());
        auto property = find.value();
        _setter = [property](const QVariant& value){
            property->SetValue(value);
        };
        property->SetValue(value);
    })
{

}

PropertyPromiseBase::PropertyPromiseBase(const Name& name, const Property::FOnChange& onChange, qint32 contextIndex)
    : PropertyPromiseBase(name, contextIndex)
{
    PropertiesSystem::Subscribe(name, onChange);
}
