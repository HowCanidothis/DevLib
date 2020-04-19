#include "propertiessystem.h"

#include <QSettings>

#include <SharedModule/stack.h>
#include <SharedModule/Threads/threadeventshelper.h>

#include "property.h"
#include "propertiesscope.h"

const Name PropertiesSystem::Global = Name("GlobalScope");
const Name PropertiesSystem::InitProperties = Name("InitPropertiesScope");
const Name PropertiesSystem::Temp = Name("TempScope");

PropertiesScope* PropertiesSystem::GetScope(const PropertiesScopeName& scope)
{
    return getOrCreateScope(scope);
}

PropertiesScope* PropertiesSystem::GetCurrentScope()
{
    return currentScope();
}

void PropertiesSystem::Subscribe(const Name& path, const FAction& function)
{
    currentScope()->Subscribe(path, function);
}

void PropertiesSystem::Subscribe(const FAction& function)
{
    currentScope()->Subscribe(function);
}

void PropertiesSystem::ForeachProperty(const std::function<void (Property*)>& handle)
{
    currentScope()->ForeachProperty(handle);
}

QVariant PropertiesSystem::GetValue(const Name& path)
{
    return currentScope()->GetValue(path);
}

bool PropertiesSystem::Load(const QString& fileName, const PropertiesScopeName& scope)
{
    return getOrCreateScope(scope)->Load(fileName);
}

void PropertiesSystem::Save(const QString& fileName, const PropertiesScopeName& scope)
{
    getOrCreateScope(scope)->Save(fileName);
}

void PropertiesSystem::Save(const QString& fileName, const PropertiesScopeName& scope, const QVector<Name>& propertyName)
{
    getOrCreateScope(scope)->Save(fileName, propertyName);
}

void PropertiesSystem::Clear(const PropertiesScopeName& scope)
{
    getOrCreateScope(scope)->Clear();
}

QVariant PropertiesSystem::GetValue(const Name& path, const PropertiesScopeName& scope)
{
    return getOrCreateScope(scope)->GetValue(path);
}

PropertiesSystem::FHandle& PropertiesSystem::Begin(const PropertiesScopeName& scope)
{
    Q_ASSERT(currentScope()->GetName() == Global);
    currentScope() = getOrCreateScope(scope);
    return currentScope()->Begin();
}

void PropertiesSystem::Begin(ThreadEventsContainer* thread, const PropertiesScopeName& scope)
{
    Q_ASSERT(currentScope()->GetName() == Global);
    currentScope() = getOrCreateScope(scope);
    return currentScope()->Begin(thread);
}

void PropertiesSystem::End()
{
    currentScope()->End();
    currentScope() = getOrCreateScope(Global);
}

void PropertiesSystem::addProperty(Name path, Property* property)
{
    currentScope()->addProperty(path, property);
}

PropertiesScope* PropertiesSystem::getOrCreateScope(const PropertiesScopeName& scope)
{
    auto foundIt = scopes().find(scope);
    PropertiesScope* res;
    if(foundIt == scopes().end()) {
        res = new PropertiesScope(scope);
        scopes().insert(scope, res);
    } else {
        res = foundIt.value();
    }
    return res;
}

QHash<Name, PropertiesScope*>& PropertiesSystem::scopes()
{
    static QHash<Name, PropertiesScope*> result;
    return result;
}

PropertiesScope*& PropertiesSystem::currentScope()
{
    static PropertiesScope* res = getOrCreateScope(Global);
    return res;
}

PropertiesSystemScopeGuard::PropertiesSystemScopeGuard(const PropertiesScopeName& scope) Q_DECL_NOEXCEPT
{
    PropertiesSystem::Begin(scope);
}

PropertiesSystemScopeGuard::~PropertiesSystemScopeGuard()
{
    PropertiesSystem::End();
}
