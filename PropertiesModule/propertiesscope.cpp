#include "propertiesscope.h"

#include <QSettings>

#include "propertiessystem.h"

PropertiesScope::FHandle& PropertiesScope::defaultHandle()
{
    static FHandle result = [](const FAction& s){ s(); };
    return result;
}

void PropertiesScope::addProperty(Name path, Property* property)
{
    Q_ASSERT_X(!m_properties.contains(path), "PropertiesSystem::addProperty", path.AsString().toLatin1().constData());
    property->Handler() = m_currentHandle;
    m_properties.insert(path, property);
    auto findSubscribes = m_delayedSubscribes.find(path);
    if(findSubscribes != m_delayedSubscribes.end()) {
        auto subscribes = findSubscribes.value();
        property->Subscribe([subscribes]{
            for(auto subscribe : subscribes) {
                subscribe();
            }
        });
        m_delayedSubscribes.remove(path);
    }
}

PropertiesScope::PropertiesScope(const PropertiesScopeName& name)
    : m_name(name)
    , m_currentHandle(defaultHandle())
{

}

void PropertiesScope::SetValueForceInvoke(const Name& path, const QVariant& value)
{
    auto find = m_properties.find(path);
    Q_ASSERT_X(find != m_properties.end(), "PropertiesSystem::setValue", path.AsString().toLatin1().constData());
    if(!find.value()->SetValue(value)) {
        find.value()->Invoke();
    }
}

void PropertiesScope::SetValue(const Name& path, const QVariant& value)
{
    auto find = m_properties.find(path);
    Q_ASSERT_X(find != m_properties.end(), "PropertiesSystem::setValue", path.AsString().toLatin1().constData());
    find.value()->SetValue(value);
}

void PropertiesScope::Subscribe(const Name& path, const FAction& function)
{
    auto find = m_properties.find(path);
    if(find == m_properties.end()) {
        auto delayedSubscribesFind = m_delayedSubscribes.find(path);
        if(delayedSubscribesFind != m_delayedSubscribes.end()) {
            delayedSubscribesFind.value().append(function);
        } else {
            m_delayedSubscribes.insert(path, {function});
        }
    } else {
        find.value()->Subscribe(function);
    }
}

void PropertiesScope::Subscribe(const FAction& function)
{
    for(Property* property : m_properties) {
        property->Subscribe(function);
    }
}

void PropertiesScope::ForeachProperty(const std::function<void (Property*)>& handle)
{
    for(Property* property : m_properties) {
        handle(property);
    }
}

QVariant PropertiesScope::GetValue(const Name& path) const
{
    auto find = m_properties.find(path);
    Q_ASSERT_X(find != m_properties.end(), "PropertiesSystem::getValue", path.AsString().toLatin1().constData());
    return find.value()->getValue();
}

Stack<Property*> PropertiesScope::AllProperties() const
{
    Stack<Property*> result;
    for(Property* property : m_properties) {
        result.Append(property);
    }
    return result;
}

bool PropertiesScope::Load(const QString& fileName)
{
    Q_ASSERT(!fileName.isEmpty());
    if(!QFile::exists(fileName))
    {
        return false;
    }
    QSettings settings(fileName, QSettings::IniFormat);
    settings.setIniCodec("utf-8");

    const auto& tree = m_properties;

    for(const QString& key : settings.allKeys()) {
        auto find = tree.find(Name(key));
        if(find == tree.end()) {
            qCWarning(LC_SYSTEM) << "unknown property" << key;
        } else {
            if(find.value()->GetOptions().TestFlag(Property::Option_IsExportable)) {
                if(!find.value()->SetValue(settings.value(key))) {
                    find.value()->Invoke();
                }
            }
        }
    }
    return true;
}

void PropertiesScope::Save(const QString& fileName)
{
    Q_ASSERT(!fileName.isEmpty());
    QSettings settings(fileName, QSettings::IniFormat);
    settings.setIniCodec("utf-8");

    for(auto* property : m_properties) {
        if(property->GetOptions().TestFlag(Property::Option_IsExportable)) {
            settings.setValue(property->GetPropertyName().AsString(), property->getValue());
        }
    }
}

void PropertiesScope::Save(const QString& fileName, const QVector<Name>& propertyNames)
{
    Q_ASSERT(!fileName.isEmpty());
    QSettings settings(fileName, QSettings::IniFormat);
    settings.setIniCodec("utf-8");

    auto& container = m_properties;

    for(const auto& name : propertyNames) {
        auto foundIt = container.find(name);
        Q_ASSERT(foundIt != container.end());
        Q_ASSERT(foundIt.value()->GetOptions().TestFlag(Property::Option_IsExportable));

        settings.setValue(foundIt.key().AsString(), foundIt.value()->getValue());
    }
}

void PropertiesScope::Clear()
{
    Q_ASSERT(m_name != PropertiesSystem::Global);
    for(auto property : m_properties) {
        delete property;
    }
    m_properties.clear();
}

void PropertiesScope::ClearWithoutDeleting()
{
    Q_ASSERT(m_name != PropertiesSystem::Global);
    m_properties.clear();
}

PropertiesScope::FHandle& PropertiesScope::Begin()
{
    return m_currentHandle;
}

void PropertiesScope::Begin(ThreadEventsContainer* thread)
{
    Begin() = [thread](const FAction& setter){ thread->Asynch(setter); };
}

void PropertiesScope::End()
{
    m_currentHandle = defaultHandle();
}

bool PropertiesScope::IsExists(const Name& path) const
{
    return m_properties.find(path) != m_properties.end();
}
