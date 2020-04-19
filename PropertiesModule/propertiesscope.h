#ifndef PROPERTIESSCOPE_H
#define PROPERTIESSCOPE_H

#include <SharedModule/internal.hpp>

#include "propertypromise.h"

class Property;

class _Export PropertiesScope
{
public:
    using FHandle = PropertiesSystem::FHandle;

    PropertiesScope(const PropertiesScopeName& name);

    const PropertiesScopeName& GetName() const { return m_name; }
    void SetValueForceInvoke(const Name &path, const QVariant &value);
    void SetValue(const Name& path, const QVariant& value);
    void Subscribe(const Name& path, const FAction& function);
    void Subscribe(const FAction& function);
    void ForeachProperty(const std::function<void (Property*)>& handle);
    QVariant GetValue(const Name& path) const;
    template<class T>
    PropertyPromise<T> GetProperty(const Name& path) const;

    bool Load(const QString& fileName);
    void Save(const QString& fileName);
    void Save(const QString& fileName, const QVector<Name>& propertyNames);

    void Clear();
    void ClearWithoutDeleting();

    // begin current context. Global <= type < Max
    // return FHandle reference. It is property setter by default it just call SetValue()
    // every property created between Begin - End uses setted handle
    // change it for example for thread safety.
    // Example:
    // handle = [threadWherePropertyIs](const auto& setter){ threadWherePropertyIs->Asynch(setter); }
    FHandle& Begin();
    // convenient Begin overload. Use it when property exists in different from the main thread
    void Begin(class ThreadEventsContainer* thread);
    void End();

    bool IsExists(const Name& path) const;

private:
    friend class PropertiesSystem;
    friend class PropertyPromiseBase;
    friend class PropertiesModel;

    static FHandle& defaultHandle();
    void addProperty(Name path, Property* property);

private:
    Q_DISABLE_COPY(PropertiesScope)
    QHash<Name, Property*> m_properties;
    FHandle m_currentHandle;
    QHash<Name, QVector<Property::FOnChange>> m_delayedSubscribes;
    PropertiesScopeName m_name;
};

template<class T>
PropertyPromise<T> PropertiesScope::GetProperty(const Name& path) const
{

}

#endif // PROPERTIESSCOPE_H
