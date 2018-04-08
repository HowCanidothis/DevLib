#include "propertiessystem.h"
#include "property.h"
#include "Shared/stack.h"

void PropertiesSystem::setValue(const QString& path, const QVariant& value)
{
    auto find = context().find(path);
    Q_ASSERT_X(find == context().end(), "PropertiesSystem::setValue", path.toLatin1().constData());
    find.value()->setValue(value);
}

QVariant PropertiesSystem::getValue(const QString& path)
{
    auto find = context().find(path);
    Q_ASSERT_X(find != context().end(), "PropertiesSystem::getValue", path.toLatin1().constData());
    return find.value()->getValue();
}

PropertiesSystem::FHandle& PropertiesSystem::begin(Type type)
{
    static FHandle res = defaultHandle();
    Q_ASSERT(type >= 0 && type < Max);

    return res;
}

void PropertiesSystem::end()
{
    begin() = defaultHandle();
    currentType() = Global;
}

void PropertiesSystem::addProperty(const QString& path, Property* property) {

    Q_ASSERT_X(!context().contains(path), "PropertiesSystem::setValue", path.toLatin1().constData());
    property->handle() = begin();
    context().insert(path, property);
}

QHash<QString, Property*>& PropertiesSystem::context()
{
    static StackPointers<QHash<QString, Property*>> res(Max); return *res[currentType()];
}

PropertiesSystem::FHandle PropertiesSystem::defaultHandle()
{
    return [](const FSetter& s){ s(); };
}

PropertiesSystem::Type& PropertiesSystem::currentType()
{
    static Type res = Global; return res;
}
