#ifndef PROPS_H
#define PROPS_H

#include <functional>
#include <QHash>
#include <QVariant>

#include <SharedModule/internal.hpp>

class Property;
class Name;
class StateProperty;
class PropertiesScope;
template<class T> class PropertyPromise;

using PropertiesScopeName = Name;


class PropertiesSystemScopeGuard
{
public:
    explicit PropertiesSystemScopeGuard(const PropertiesScopeName& scope) Q_DECL_NOEXCEPT;
    ~PropertiesSystemScopeGuard();
};

class _Export PropertiesSystem
{
public:
    using FHandle = std::function<void (const FAction&)>;

    static const Name Global;
    static const Name InitProperties;
    static const Name Temp;

    static PropertiesScope* GetScope(const PropertiesScopeName& scope);
    static PropertiesScope* GetCurrentScope();

    static bool Load(const QString& fileName, const PropertiesScopeName& scope);
    static void Save(const QString& fileName, const PropertiesScopeName& scope);
    static void Save(const QString& fileName, const PropertiesScopeName& scope, const QVector<Name>& propertyName);
    static void Clear(const PropertiesScopeName& scope);
    template<class T>
    static PropertyPromise<T> GetProperty(const Name& path, const PropertiesScopeName& scope);
    static QVariant GetValue(const Name& path, const PropertiesScopeName& scope);

    // begin current context. Global <= type < Max
    // return FHandle reference. It is property setter by default it just call SetValue()
    // every property created between Begin - End uses setted handle
    // change it for example for thread safety.
    // Example:
    // handle = [threadWherePropertyIs](const auto& setter){ threadWherePropertyIs->Asynch(setter); }
    static FHandle& Begin(const PropertiesScopeName& scope=Global);
    // convenient Begin overload. Use it when property exists in different from the main thread
    static void Begin(class ThreadEventsContainer* thread, const PropertiesScopeName& scope=Global);
    static void End();

    static void Subscribe(const Name& path, const FAction& function);
    static void Subscribe(const FAction& function);
    static void ForeachProperty(const std::function<void (Property*)>& handle);
    static QVariant GetValue(const Name& path);

private:
    friend class Property;
    friend class PropertiesModel;
    friend class PropertyPromiseBase;

    PropertiesSystem();
    Q_DISABLE_COPY(PropertiesSystem)

    static void addProperty(Name path, Property* property);

    static class PropertiesScope* getOrCreateScope(const PropertiesScopeName& scope);

    static QHash<Name, class PropertiesScope*>& scopes();
    static PropertiesScope*& currentScope();
};

#endif // PROPS_H
