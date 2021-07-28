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
    DefaultFactoryBase()
        : m_defaultDelegate([] { return nullptr; })
    {}

    DelegateObject* Create(const QString& extension) const;
    ScopedPointer<DelegateObject> CreateScoped(const QString& extension) const {
        return ScopedPointer<DelegateObject>(Create(extension));
    }

    bool IsSupport(const QString& extension) const { return m_delegates.contains(extension); }
    QString GetSupportedExtensions(const QString& suffix) const;
	QStringList GenerateSupportedExtensions(const std::function<QString(const QString&)>& func = [](const QString& str) {return QString("%1 (*.%2)").arg(str.toUpper(), str.toLower()); }) const;
	
protected:
    void associate(const QString& formats, const DelegateCreator& importerCreator);
    void setDefault(const DelegateCreator& importerCreator);

private:
    Delegates m_delegates;
    DelegateCreator m_defaultDelegate;
};

template<class DelegateObject>
DelegateObject* DefaultFactoryBase<DelegateObject>::Create(const QString& fileExtension) const
{
    auto find = m_delegates.find(fileExtension.toLower());
    if(find == m_delegates.end()) {
        return m_defaultDelegate();
    }
    return find.value()();
}

template<class DelegateObject>
QStringList DefaultFactoryBase<DelegateObject>::GenerateSupportedExtensions(const std::function<QString(const QString&)>& func) const
{
	QStringList result;
    auto it = m_delegates.begin();
    auto e = m_delegates.end();
    for(; it != e; it++) {
        result.append(func(it.key()));
    }
    return result;
}

template<class DelegateObject>
QString DefaultFactoryBase<DelegateObject>::GetSupportedExtensions(const QString& suffix) const
{
    QString result;
    auto it = m_delegates.begin();
    auto e = m_delegates.end();
    for(; it != e; it++) {
        result += suffix + it.key() + " ";
    }
    return result;
}

template<class DelegateObject>
void DefaultFactoryBase<DelegateObject>::associate(const QString& formats, const DelegateCreator& importerCreator)
{
    for(const auto& format : formats.split(" ", QString::SkipEmptyParts)) {
        m_delegates.insert(format, importerCreator);
    }
}

template<class DelegateObject>
void DefaultFactoryBase<DelegateObject>::setDefault(const DelegateCreator& importerCreator)
{
    m_defaultDelegate = importerCreator;
}

#endif // DEFAULTFACTORYBASE_H
