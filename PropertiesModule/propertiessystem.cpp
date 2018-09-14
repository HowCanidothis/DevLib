#include "propertiessystem.h"

#include <QSettings>

#include <SharedModule/stack.h>
#include <SharedModule/threads/threadeventshelper.h>

#include "property.h"

static StackPointers<QHash<Name, Property*>>& contexts()
{
    static StackPointers<QHash<Name, Property*>> res(PropertiesSystem::Max);
    return res;
}

static QHash<Name, QVector<Property::FOnChange>>& delayedSubscribes()
{
    static QHash<Name, QVector<Property::FOnChange>> res;
    return res;
}

void PropertiesSystem::SetValueForceInvoke(const Name& path, const QVariant& value)
{
    auto find = context().find(path);
    Q_ASSERT_X(find != context().end(), "PropertiesSystem::setValue", path.AsString().toLatin1().constData());
    if(!find.value()->SetValue(value)) {
        find.value()->Invoke();
    }
}

void PropertiesSystem::SetValue(const Name& path, const QVariant& value)
{
    auto find = context().find(path);
    Q_ASSERT_X(find != context().end(), "PropertiesSystem::setValue", path.AsString().toLatin1().constData());
    find.value()->SetValue(value);
}

void PropertiesSystem::Subscribe(const Name& path, const PropertiesSystem::FOnChange& function)
{
    auto find = context().find(path);
    if(find == context().end()) {
        auto delayedSubscribesFind = delayedSubscribes().find(path);
        if(delayedSubscribesFind != delayedSubscribes().end()) {
            delayedSubscribesFind.value().append(function);
        } else {
            delayedSubscribes().insert(path, {function});
        }
    } else {
        find.value()->Subscribe(function);
    }
}

void PropertiesSystem::ForeachProperty(const std::function<void (Property*)>& handle)
{
    ForeachProperty(handle, currentContextIndex());
}

void PropertiesSystem::ForeachProperty(const std::function<void (Property*)>& handle, qint32 contextIndex)
{
    for(Property* property : context(contextIndex)) {
        handle(property);
    }
}

void PropertiesSystem::Subscribe(const PropertiesSystem::FOnChange& function)
{
    for(Property* property : context()) {
        property->Subscribe(function);
    }
}

QVariant PropertiesSystem::GetValue(const Name& path)
{
    auto find = context().find(path);
    Q_ASSERT_X(find != context().end(), "PropertiesSystem::getValue", path.AsString().toLatin1().constData());
    return find.value()->getValue();
}

QVariant PropertiesSystem::GetValue(const Name& path, qint32 type)
{
    auto find = contexts()[type]->find(path);
    Q_ASSERT_X(find != contexts()[type]->end(), "PropertiesSystem::getValue", path.AsString().toLatin1().constData());
    return find.value()->getValue();
}

void PropertiesSystem::Load(const QString& fileName, properties_context_index_t contextIndex)
{
    Q_ASSERT(!fileName.isEmpty());
    QSettings settings(fileName, QSettings::IniFormat);
    settings.setIniCodec("utf-8");

    const auto& tree = PropertiesSystem::context(contextIndex);

    for(const QString& key : settings.allKeys()) {
        auto find = tree.find(Name(key));
        if(find == tree.end()) {
            qCWarning(LC_SYSTEM) << "unknown property" << key;
        } else {
            if(find.value()->GetOptions().TestFlag(Property::Option_IsExportable)) {
                find.value()->SetValue(settings.value(key));
            }
        }
    }
}

void PropertiesSystem::Save(const QString& fileName, properties_context_index_t contextIndex)
{
    Q_ASSERT(!fileName.isEmpty());
    QSettings settings(fileName, QSettings::IniFormat);
    settings.setIniCodec("utf-8");

    auto it = context(contextIndex).begin();
    auto e = context(contextIndex).end();
    for(; it != e; it++) {
        if(it.value()->GetOptions().TestFlag(Property::Option_IsExportable)) {
            settings.setValue(it.key().AsString(), it.value()->getValue());
        }
    }
}

void PropertiesSystem::Clear()
{
    Clear(currentContextIndex());
}

void PropertiesSystem::Clear(qint32 contextIndex)
{
    Q_ASSERT(contextIndex != Global);
    auto& context = PropertiesSystem::context(contextIndex);
    for(auto property : context) {
        delete property;
    }
    context.clear();
}

bool PropertiesSystem::HasContext(qint32 contextIndex)
{
    return !context(contextIndex).isEmpty();
}

properties_context_index_t PropertiesSystem::GetCurrentContextIndex()
{
    return currentContextIndex();
}

PropertiesSystem::FHandle& PropertiesSystem::Begin(Type type)
{
    Q_ASSERT(type >= 0 && type < Max);
    currentHandle() = defaultHandle();
    currentContextIndex() = type;

    return currentHandle();
}

void PropertiesSystem::Begin(ThreadEventsContainer* thread, PropertiesSystem::Type type)
{
    Begin(type) = [thread](const auto& setter){ thread->Asynch(setter); };
}

void PropertiesSystem::End()
{
    currentHandle() = defaultHandle();
    currentContextIndex() = Global;
}

void PropertiesSystem::addProperty(const Name& path, Property* property) {

    Q_ASSERT_X(!context().contains(path), "PropertiesSystem::addProperty", path.AsString().toLatin1().constData());
    property->Handler() = currentHandle();
    context().insert(path, property);
    auto findSubscribes = delayedSubscribes().find(path);
    if(findSubscribes != delayedSubscribes().end()) {
        auto subscribes = findSubscribes.value();
        property->Subscribe([subscribes]{
            for(auto subscribe : subscribes) {
                subscribe();
            }
        });
        delayedSubscribes().remove(path);
    }
}

QHash<Name, Property*>& PropertiesSystem::context(properties_context_index_t contextIndex)
{
    return *contexts()[contextIndex];
}

QHash<Name, Property*>& PropertiesSystem::context()
{
    return *contexts()[currentContextIndex()];
}

PropertiesSystem::FHandle PropertiesSystem::defaultHandle()
{
    return [](const FSetter& s){ s(); };
}

PropertiesSystem::FHandle& PropertiesSystem::currentHandle()
{
    static FHandle res = defaultHandle();
    return res;
}

PropertiesSystem::Type& PropertiesSystem::currentContextIndex()
{
    static Type res = Global; return res;
}
