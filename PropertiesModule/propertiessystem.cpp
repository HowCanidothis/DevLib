#include "propertiessystem.h"

#include <QSettings>

#include <SharedModule/stack.h>
#include <SharedModule/Threads/threadeventshelper.h>

#include "property.h"
#include "propertiesscope.h"

const Name PropertiesSystem::Global = Name("GlobalScope");
const Name PropertiesSystem::InitProperties = Name("InitPropertiesScope");
const Name PropertiesSystem::Temp = Name("TempScope");
const Name PropertiesSystem::Empty = Name("EmptyScope");

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

bool PropertiesSystem::IsExists(const Name& path, const PropertiesScopeName& scope)
{
    return getOrCreateScope(scope)->IsExists(path);
}

void PropertiesSystem::SetValue(const Name& path, const QVariant& value)
{
    currentScope()->SetValue(path, value);
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

void PropertiesSystem::ClearWithoutDeleting(const PropertiesScopeName& scope)
{
    getOrCreateScope(scope)->ClearWithoutDeleting();
}

QVariant PropertiesSystem::GetValue(const Name& path, const PropertiesScopeName& scope)
{
    return getOrCreateScope(scope)->GetValue(path);
}

PropertiesSystem::FHandle& PropertiesSystem::Begin(const PropertiesScopeName& scope)
{
    THREAD_ASSERT_IS_MAIN()
    currentScope() = getOrCreateScope(scope);
    scopesDepth().Append(currentScope());
    return currentScope()->Begin();
}

void PropertiesSystem::Begin(ThreadEventsContainer* thread, const PropertiesScopeName& scope)
{
    THREAD_ASSERT_IS_MAIN()
    currentScope() = getOrCreateScope(scope);
    scopesDepth().Append(currentScope());
    return currentScope()->Begin(thread);
}

void PropertiesSystem::End()
{
    THREAD_ASSERT_IS_MAIN()
    Q_ASSERT(scopesDepth().Size() > 1);
    currentScope()->End();
    scopesDepth().Pop();
    currentScope() = scopesDepth().Last();
}

void PropertiesSystem::BeginPrefix(const QString& prefix)
{
    THREAD_ASSERT_IS_MAIN()
    auto newPrefix = prefixesDepth().last() + prefix;
    prefixesDepth().append(newPrefix);
    currentPrefix() = newPrefix;
}

void PropertiesSystem::EndPrefix()
{
    THREAD_ASSERT_IS_MAIN()
    Q_ASSERT(prefixesDepth().size() > 1);
    prefixesDepth().pop_back();
    currentPrefix() = prefixesDepth().last();
}

void PropertiesSystem::addProperty(Name path, Property* property)
{
    THREAD_ASSERT_IS_MAIN()

    if(!currentPrefix().isEmpty()) {
        path.SetName(currentPrefix() + path.AsString());
        property->m_propertyName = path;
    }
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

Stack<PropertiesScope*>& PropertiesSystem::scopesDepth()
{
    static Stack<PropertiesScope*> result { getOrCreateScope(PropertiesSystem::Global) };
    return result;
}

QString& PropertiesSystem::currentPrefix()
{
    static QString result;
    return result;
}

QVector<QString>& PropertiesSystem::prefixesDepth()
{
    static QVector<QString> result{ "" };
    return result;
}

PropertiesSystemScopeGuard::PropertiesSystemScopeGuard(const PropertiesScopeName& scope) Q_DECL_NOEXCEPT
{
    PropertiesSystem::Begin(scope);
}

PropertiesSystemScopeGuard::~PropertiesSystemScopeGuard()
{
    PropertiesSystem::End();
}
