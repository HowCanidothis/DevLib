#include "name.h"
#include <QHashFunctions>

#ifdef QT_DEBUG
#define ADD_NAME if(DebugNameManager::IsInitialized()) DebugNameManager::GetInstance().addName(AsString());
#define REMOVE_NAME if(DebugNameManager::IsInitialized()) DebugNameManager::GetInstance().removeName(AsString());

DebugNameManager::DebugNameManager()
{
}

void DebugNameManager::PrintReport(qint32 maxSymbolUsage)
{
    QMutexLocker locker(&m_mutex);
    auto totalLostSize = 0;
    for(auto it(m_counter.cbegin()), e(m_counter.cend()); it != e; ++it) {
        auto count = 0;
        for(const auto& value : it.value()) {
            if(value != 0) {
                ++count;
            }
        }
        auto tookBytes = it.key().size() * count;
        if(tookBytes > maxSymbolUsage) {
            qDebug().noquote() << QString("\"%1\":Possible extra memory usage, took %2 symbols with %3 copies").arg(it.key(), QString::number(tookBytes), QString::number(count));
        }
        if(count > 1) {
            totalLostSize += it.key().size() * (count - 1);
        }
    }
    qDebug() << QString("==========================================Total Lost Symbols: %1================================").arg(totalLostSize);
}

void DebugNameManager::addName(const QString& name)
{
    if(this == nullptr || name.isEmpty()) {
        return;
    }
    QMutexLocker locker(&m_mutex);
    auto foundIt = m_counter.find(name);
    if(foundIt == m_counter.end()) {
        foundIt = m_counter.insert(name, QHash<const QChar*,qint32>());
    }
    auto foundIt2 = foundIt->find(name.data());
    if(foundIt2 == foundIt->end()) {
        foundIt2 = foundIt->insert(name.data(), 0);
    }
    auto& counter = foundIt2.value();
    ++counter;
}

void DebugNameManager::removeName(const QString& name)
{
    if(this == nullptr || name.isEmpty()) {
        return;
    }
    QMutexLocker locker(&m_mutex);
    auto foundIt = m_counter.find(name);
    if(foundIt != m_counter.end()) {
        auto foundIt2 = foundIt->find(name.data());
        if(foundIt2 != foundIt->end()) {
            auto& counter = foundIt2.value();
            --counter;
        }
    }
}
#else
#define ADD_NAME
#define REMOVE_NAME
#endif

UniName::UniName(const char* name)
    : m_name(name)
    , m_latinName(name)
{
}

UniName::UniName(const Latin1Name& name)
    : m_name(QString::fromStdString(name.AsLatin1String()))
    , m_latinName(name)
{

}

UniName::UniName(const Name& name)
    : m_name(name)
    , m_latinName(name.AsString().toStdString())
{

}

UniName::UniName(const std::string& name)
    : m_name(QString::fromStdString(name))
    , m_latinName(name)
{

}

UniName::UniName(std::string&& name)
    : m_name(QString::fromStdString(name))
    , m_latinName(std::move(name))
{

}

UniName::UniName(const QString& name)
    : m_name(name)
    , m_latinName(name.toStdString())
{

}

void UniName::SetName(const std::string& name)
{
    m_name.SetName(QString::fromStdString(name));
    m_latinName.SetName(name);
}

void UniName::SetName(const QString& name)
{
    m_name.SetName(name);
    m_latinName.SetName(name.toStdString());
}

Name::Name()
    : m_value(0)
{

}

Name::Name(const char* name)
    : m_text(name)
    , m_value(qHash(m_text))

{
    ADD_NAME
}

Name Name::MakeUniqueName(const Name& name, const QSet<Name>& names)
{
    if(!names.contains(name)) {
        return name;
    }

    thread_local static QRegExp regexp(R"(\((\d+)\)$)");
    Name result;
    auto cr = name.AsString();

    if(regexp.indexIn(cr) != -1) {
        auto decimalPart = regexp.cap(1);
        auto oldDecimalPart = decimalPart.toInt();
        cr.replace(regexp, QString("(%1)").arg(oldDecimalPart + 1));
        result = MakeUniqueName(Name(cr), names);
    } else {
        cr += " (1)";
        result = MakeUniqueName(Name(cr), names);
    }

    return result;
}

Name::Name(const Name& other)
    : m_text(other.m_text)
    , m_value(other.m_value)
{
    ADD_NAME
}

Name::Name(const QString& name)
    : m_text(name)
    , m_value(qHash(m_text))
{
    ADD_NAME
}

Name::~Name()
{
    REMOVE_NAME
}

Name& Name::operator=(const Name& another)
{
    REMOVE_NAME
    m_value = another.m_value;
    m_text = another.m_text;
    ADD_NAME
    return *this;
}

void Name::SetName(const QString& str)
{
    REMOVE_NAME
    m_value = qHash(str);
    m_text = str;
    ADD_NAME
}

Name Name::Joined(const std::initializer_list<Name>& names)
{
    QString result;
    for(const auto& name : names) {
        result += name.AsString();
    }
    return Name(result);
}

Name Name::Joined(const QString& string) const
{
    return Name(AsString() + string);
}

const QString& Name::AsString() const
{
    return m_text;
}

Latin1Name::Latin1Name()
    : m_text(new std::string)
    , m_value(0)
{

}

Latin1Name::Latin1Name(const char* name)
    : m_text(new std::string(name))
    , m_value(qHashBits(m_text->data(), m_text->size()))
{

}

Latin1Name::Latin1Name(std::string&& name)
    : m_text(new std::string(std::move(name)))
    , m_value(qHashBits(m_text->data(), m_text->size()))
{

}

void Latin1Name::SetName(const std::string& name)
{
    m_value = qHashBits(name.data(), name.size());
    *m_text = name;
}

Latin1Name::Latin1Name(const std::string& name)
    : m_text(new std::string(name))
    , m_value(qHashBits(name.data(), name.size()))
{

}
