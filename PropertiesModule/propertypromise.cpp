#include "propertypromise.h"

PropertyPromiseBase::PropertyPromiseBase()
    : m_isValid([]{
        return false;
    })
{

}

PropertyPromiseBase::PropertyPromiseBase(const Name& name, qint32 contextIndex)
    : m_getter([name, contextIndex, this]{
        auto context = PropertiesSystem::context(contextIndex);
        auto find = context.find(name);
        Q_ASSERT_X(find != context.end(), "PropertiesSystem::getValue", name.AsString().toLatin1().constData());
        auto property = find.value();
        m_getter = [property]{ return property; };
        return property;
    })
    , m_isValid([name, contextIndex, this]{
        auto context = PropertiesSystem::context(contextIndex);
        if(context.contains(name)) {
            m_isValid = []{ return true; };
        } else {
            return false;
        }
        return true;
    })
    , m_setter([name, contextIndex, this](const QVariant& value){
        auto context = PropertiesSystem::context(contextIndex);
        auto find = context.find(name);
        Q_ASSERT_X(find != context.end(), "PropertiesSystem::setValue", name.AsString().toLatin1().constData());
        auto property = find.value();
        m_setter = [property](const QVariant& value){
            property->SetValue(value);
        };
        property->SetValue(value);
    })
    , m_propertyName(name)
{

}

PropertyPromiseBase::PropertyPromiseBase(const Name& name, const Property::FOnChange& onChange, qint32 contextIndex)
    : PropertyPromiseBase(name, contextIndex)
{
    PropertiesSystem::Subscribe(name, onChange);
}

void PropertyPromiseBase::Subscribe(const Property::FOnChange& onChange)
{
    if(IsValid()) {
        GetProperty()->Subscribe(onChange);
    } else {
        PropertiesSystem::Subscribe(m_propertyName, onChange);
    }
}
