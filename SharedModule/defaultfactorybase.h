#ifndef DEFAULTFACTORYBASE_H
#define DEFAULTFACTORYBASE_H

#include <QHash>
#include <functional>
#include "smartpointersadapters.h"
#include "shared_decl.h"

template<class DelegateObject>
class DefaultFactoryBase
{
    typedef std::function<DelegateObject* ()> DelegateCreator;
    typedef QHash<QString, DelegateCreator> Delegates;

public:
    static DelegateObject* Create(const QString& extension);
    static ScopedPointer<DelegateObject> CreateScoped(const QString& extension) {
        return ScopedPointer<DelegateObject>(Create(extension));
    }

    static bool IsSupport(const QString& extension) { return delegates().contains(extension); }
    static QString GetSupportedExtensions(const QString& suffix);

protected:
    static void associate(const QString& formats, const DelegateCreator& importerCreator);
    static void setDefault(const DelegateCreator& importerCreator);

private:
    static Delegates& delegates()
    {
        static Delegates ret;
        return ret;
    }
    static DelegateCreator& defaultDelegate()
    {
        static DelegateCreator ret;
        return ret;
    }
};

template<class DelegateObject>
DelegateObject* DefaultFactoryBase<DelegateObject>::Create(const QString& fileExtension)
{
    auto find = delegates().find(fileExtension.toLower());
    if(find == delegates().end()) {
         return defaultDelegate()();
    }
    return find.value()();
}

template<class DelegateObject>
QString DefaultFactoryBase<DelegateObject>::GetSupportedExtensions(const QString& suffix)
{
    QString result;
    auto it = delegates().begin();
    auto e = delegates().end();
    for(; it != e; it++) {
        result += suffix + it.key() + " ";
    }
    return result;
}

template<class DelegateObject>
void DefaultFactoryBase<DelegateObject>::associate(const QString& formats, const DelegateCreator& importerCreator)
{
    for(const auto& format : formats.split(" ", QString::SkipEmptyParts)) {
        delegates().insert(format, importerCreator);
    }
}

template<class DelegateObject>
void DefaultFactoryBase<DelegateObject>::setDefault(const DelegateCreator& importerCreator)
{
    defaultDelegate() = importerCreator;
}

#endif // DEFAULTFACTORYBASE_H
