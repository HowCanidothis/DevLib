#include "propertypromise.h"

#include "propertiesscope.h"

PropertyPromiseBase::PropertyPromiseBase()
    : m_isValid([]{
        return false;
    })
{

}

PropertyPromiseBase::PropertyPromiseBase(const Name& name, const Name& scopeName)
{
    Assign(name, scopeName);
}

PropertyPromiseBase::PropertyPromiseBase(Property* property)
{
    m_getter = [property] { return property; };
    m_isValid = [] { return true; };
    m_setter = [property](const QVariant& value) {
        property->SetValue(value);
    };
    m_propertyName = property->GetPropertyName();
}

void PropertyPromiseBase::Assign(const Name& name, const PropertiesScopeName& scopeName)
{
    m_getter = ([name, scopeName, this]{
            const auto& scopeProperties = PropertiesSystem::getOrCreateScope(scopeName)->m_properties;
            auto foundIt = scopeProperties.find(name);
            Q_ASSERT_X(foundIt != scopeProperties.end(), "PropertiesSystem::getValue", name.AsString().toLatin1().constData());
            auto property = foundIt.value();
            m_getter = [property]{ return property; };
            return property;
        });
    m_isValid = ([name, scopeName, this]{
            const auto& scopeProperties = PropertiesSystem::getOrCreateScope(scopeName)->m_properties;
            if(scopeProperties.contains(name)) {
                m_isValid = []{ return true; };
            } else {
                return false;
            }
            return true;
        });
    m_setter = ([name, scopeName, this](const QVariant& value){
            const auto& scopeProperties = PropertiesSystem::getOrCreateScope(scopeName)->m_properties;
            auto foundIt = scopeProperties.find(name);
            Q_ASSERT_X(foundIt != scopeProperties.end(), "PropertiesSystem::setValue", name.AsString().toLatin1().constData());
            auto property = foundIt.value();
            m_setter = [property](const QVariant& value){
                property->SetValue(value);
            };
            property->SetValue(value);
        });
    m_propertyName = name;
}

void PropertyPromiseBase::Subscribe(const Property::FOnChange& onChange)
{
    if(IsValid()) {
        GetProperty()->Subscribe(onChange);
    } else {
        PropertiesSystem::Subscribe(m_propertyName, onChange);
    }
}
