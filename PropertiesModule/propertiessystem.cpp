#include "propertiessystem.h"
#include "property.h"
#include "SharedModule/stack.h"

void PropertiesSystem::SetValue(const Name& path, const QVariant& value)
{
    auto find = context().find(path);
    Q_ASSERT_X(find != context().end(), "PropertiesSystem::setValue", path.AsString().toLatin1().constData());
    find.value()->SetValue(value);
}

QVariant PropertiesSystem::GetValue(const Name& path)
{
    auto find = context().find(path);
    Q_ASSERT_X(find != context().end(), "PropertiesSystem::getValue", path.AsString().toLatin1().constData());
    return find.value()->getValue();
}

PropertiesSystem::FHandle& PropertiesSystem::Begin(Type type)
{
    static FHandle res = defaultHandle();
    Q_ASSERT(type >= 0 && type < Max);

    return res;
}

void PropertiesSystem::End()
{
    Begin() = defaultHandle();
    currentType() = Global;
}

void PropertiesSystem::addProperty(const Name& path, Property* property) {

    Q_ASSERT_X(!context().contains(path), "PropertiesSystem::setValue", path.AsString().toLatin1().constData());
    property->Handle() = Begin();
    context().insert(path, property);
}

QHash<Name, Property*>& PropertiesSystem::context()
{
    static StackPointers<QHash<Name, Property*>> res(Max); return *res[currentType()];
}

PropertiesSystem::FHandle PropertiesSystem::defaultHandle()
{
    return [](const FSetter& s){ s(); };
}

PropertiesSystem::Type& PropertiesSystem::currentType()
{
    static Type res = Global; return res;
}
