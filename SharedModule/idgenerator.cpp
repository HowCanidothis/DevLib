#include "idgenerator.h"

namespace Id {

struct IdData
{
    IdData(class Generator* generator, void* context, const FAction& deleter)
        : Generator(generator)
        , Context(context)
        , Deleter(deleter)
    {}

    ~IdData()
    {
        Deleter();
    }

    Generator* Generator;
    void* Context;
    FAction Deleter;
};

static char IdsTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                        'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f', 'G', 'g', 'H', 'h',
                        'I', 'i', 'J', 'j', 'K', 'k', 'L', 'l', 'M', 'm', 'N', 'n', 'O', 'o', 'P', 'p',
                        'Q', 'q', 'R', 'r', 'S', 's', 'T', 't', 'U', 'u', 'V', 'v', 'W', 'w', 'X', 'x',
                        'Y', 'y', 'Z', 'z'
                      };

static Name GenerateId()
{
    static quint64 index = 1;

    auto secsSinceEpoch = QDateTime::currentDateTime().toSecsSinceEpoch();
    auto currentDateTime = QDateTime::fromSecsSinceEpoch(secsSinceEpoch ^ index);

    QString id;
    id += IdsTable[QDateTime::currentDateTime().toMSecsSinceEpoch() % std::size(IdsTable)];
    id += IdsTable[currentDateTime.time().second()];
    id += IdsTable[currentDateTime.time().minute()];
    id += IdsTable[currentDateTime.time().hour()];
    id += IdsTable[currentDateTime.date().day()];
    id += IdsTable[currentDateTime.date().month()];
    id += IdsTable[currentDateTime.date().year() % std::size(IdsTable)];
    id += IdsTable[qHash(index) % std::size(IdsTable)];
    id += IdsTable[(index / std::size(IdsTable)) % std::size(IdsTable)];
    id += IdsTable[index % std::size(IdsTable)];

    index++;
    Name result(id);
    return result;
}

static Name GenerateShortId()
{
    static quint64 index = 1;

    srand(time(0));

    QString id;
    id += IdsTable[(rand() ^ index) % std::size(IdsTable)];
    id += IdsTable[(rand() ^ index) % std::size(IdsTable)];
    id += IdsTable[qHash(index) % std::size(IdsTable)];
    id += IdsTable[(index / std::size(IdsTable)) % std::size(IdsTable)];
    id += IdsTable[index % std::size(IdsTable)];

    index++;

    return Name(id);
}

Id::Id(const Name& id)
    : Super(id)
{
}

Generator* Id::GetGenerator() const
{
    return m_data == nullptr ? nullptr : m_data->Generator;
}

Id::~Id()
{
}

void Id::Attach(Generator* generator)
{
    generator->attach(this);
}

void Id::Detach()
{
    Q_ASSERT(m_data != nullptr);
    m_data->Generator->releaseId(*this);
}

void* Id::getContext() const
{
    return m_data == nullptr ? nullptr : m_data->Context;
}

Generator::Generator(qint32 idSize)
    : m_idSize(idSize)
{
    if(idSize != 10) {
        m_generator = [this](void* context, const FAction& deleter) {
            auto result = generateComplexId();
            Q_ASSERT(result.AsString().size() == m_idSize);
            auto data = ::make_shared<IdData>(this, context, deleter);
            auto it = m_registeredIds.insert(result, data);
            return std::make_pair(it.key(), data);
        };
    } else {
        m_generator = [this](void* context, const FAction& deleter) {
            Q_ASSERT(!IsComplexId());
            while(true) {
                Name id(GenerateId());
                if(m_registeredIds.contains(id)) {
                    continue;
                }
                Q_ASSERT(id.AsString().size() == m_idSize);
                auto data = ::make_shared<IdData>(this, context, deleter);
                auto it = m_registeredIds.insert(id, data);
                return std::make_pair(it.key(), data);
            }
        };
    }
}

Id& Id::operator=(const Name& another)
{
    if(m_data != nullptr) {
        return *this;
    }
    Super::operator=(another);
    return *this;
}

Generator::~Generator()
{
    for(const auto& id : m_registeredIds) {
        auto ptr = id.lock();
        if(ptr != nullptr) {
            ptr->Deleter = []{};
        }
    }
}

Id Generator::GetId(const Name& id) const
{
    static Id defaultNull(nullptr);
    if(id.IsNull()) {
        return defaultNull;
    }

    auto foundIt = m_registeredIds.find(id);
    if(foundIt == m_registeredIds.end()) {
        return defaultNull;
    }
    auto locked = foundIt.value().lock();
    if(locked == nullptr) {
        return defaultNull;
    }
    return createId(id, static_cast<SharedPointer<IdData>&>(locked));
}

Id Generator::createId(const Name& id, const SharedPointer<IdData>& data)
{
    Id result(id);
    result.m_data = data;
    return result;
}

Id Generator::createId(void* context, const FAction& deleter)
{
    auto generated = m_generator(context, deleter);
    return createId(generated.first, generated.second);
}

void Generator::releaseId(const Name& id)
{
    auto foundIt = m_registeredIds.find(id);
    if(foundIt != m_registeredIds.end()) {
        m_registeredIds.erase(foundIt);
    }
}

void Generator::attach(Id* id, void* context, const FAction& deleter)
{
    Q_ASSERT(!id->IsNull());
    auto foundIt = m_registeredIds.find(*id);
    SharedPointer<IdData> data;
    if(foundIt == m_registeredIds.end() || foundIt.value().expired()) {
        if(id->m_data != nullptr) {
            Q_ASSERT(id->m_data->Context != nullptr);
            context = id->m_data->Context;
            id->Detach();
        } else {
            Q_ASSERT(context != nullptr);
        }
        Name idName(*id);
        data = ::make_shared<IdData>(this, context, [this, deleter, idName]{
            releaseId(idName); deleter();
        });
        foundIt = m_registeredIds.insert(*id, std::weak_ptr<IdData>(data));
    }
    id->m_data = std::shared_ptr<IdData>(foundIt.value());
}

Name Generator::generateComplexId()
{
    bool unique = false;
    Name result;
    const auto& ids = m_registeredIds;
    while(!unique) {
        auto id = GenerateShortId();
        Name fullId(BaseId.AsString() + id.AsString());
        if(ids.contains(fullId)) {
            continue;
        }
        unique = true;
        result = fullId;
    }

    return result;
}

}
