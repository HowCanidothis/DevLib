#ifndef PROPS_H
#define PROPS_H

#include <functional>
#include <QHash>
#include <QVariant>

class Property;
class Name;

class PropertiesSystem
{
public:
    enum Type {
        Global = 0,
        Max = 0xff
    };
    typedef std::function<void ()> FSetter;
    typedef std::function<void (const FSetter&)> FHandle;

    static void SetValue(const Name& path, const QVariant& value);
    static QVariant GetValue(const Name& path);

    static FHandle& Begin(Type type=Global);
    static void End();

private:
    friend class Property;
    friend class PropertiesModel;

    PropertiesSystem();
    Q_DISABLE_COPY(PropertiesSystem)

    static void addProperty(const Name& path, Property* property);

    static QHash<Name, Property*>& context();
    static FHandle defaultHandle();
    static Type& currentType();
};

#endif // PROPS_H
