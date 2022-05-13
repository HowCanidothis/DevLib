#include "idgenerator.h"

struct IdData
{
    IdData(IdGenerator* generator, const IdGenerator::AssociatedName& id);
    ~IdData();

    IdGenerator::AssociatedName Id;
    IdGenerator* Generator;
};

IdData::IdData(IdGenerator* generator, const IdGenerator::AssociatedName& id)
    : Id(id)
    , Generator(generator)
{
}

IdData::~IdData()
{
    if(!Id.IsNull()) {
        Generator->ReleaseId(Id);
    }
}

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

IdGenerator::AssociatedName::AssociatedName()
    : m_context(nullptr)
{}

IdGenerator::AssociatedName::AssociatedName(void* context, const Name& name)
    : Super(name)
    , m_context(context)
{}

IdGenerator::Id::Id(IdGenerator* generator, const AssociatedName& id)
    : m_data(new IdData(generator, id))
{

}

IdGenerator::Id::Id(IdGenerator* generator, void* context)
    : m_data(new IdData(generator, AssociatedName(context)))
{

}

bool IdGenerator::Id::IsNull() const { return m_data == nullptr ? true : m_data->Id.IsNull(); }

const QString& IdGenerator::Id::AsString() const
{
    static QString defaultResult;
    if(m_data != nullptr) {
        return m_data->Id.AsString();
    }
    return defaultResult;
}

IdGenerator::Id::operator const IdGenerator::AssociatedName&() const
{
    static AssociatedName defaultResult;
    if(m_data != nullptr) {
        return m_data->Id;
    }
    return defaultResult;
}

void* IdGenerator::Id::getContext() const
{
    return  m_data == nullptr ? nullptr : m_data->Id.As<void>();
}

IdGenerator::IdGenerator(qint32 idSize)
    : m_idSize(idSize)
{
    if(idSize > 10) {
        m_generator = [this](void* context, const AssociatedName& baseId) {
            auto result = GenerateComplexId(context, baseId, m_registeredIds);
            m_registeredIds.insert(result);
            Q_ASSERT(result.AsString().size() == m_idSize);
            return result;
        };
    } else {
        m_generator = [this](void* context, const AssociatedName&) {
            Q_ASSERT(!IsComplexId());
            while(true) {
                auto id = AssociatedName(context, ::GenerateId());
                if(m_registeredIds.contains(id)) {
                    continue;
                }
                Q_ASSERT(id.AsString().size() == m_idSize);
                m_registeredIds.insert(id);
                return id;
            }
        };
    }
}

void IdGenerator::AddId(const AssociatedName& id)
{
    Q_ASSERT(!m_registeredIds.contains(id) && id.GetSize() == m_idSize);
    m_registeredIds.insert(id);
}

IdGenerator::AssociatedName IdGenerator::RegisterId(void* context, const AssociatedName& baseId)
{
    return m_generator(context, baseId);
}

void IdGenerator::ReleaseId(const AssociatedName& id)
{
    m_registeredIds.remove(id);
}

IdGenerator::Id IdGenerator::RegisterIdObject(void* context, const AssociatedName& baseId)
{
    return Id(this,RegisterId(context, baseId));
}

IdGenerator::AssociatedName IdGenerator::GenerateComplexId(void* context, const AssociatedName& baseId, const QSet<AssociatedName>& ids)
{
    bool unique = false;
    AssociatedName result;
    while(!unique) {
        auto id = ::GenerateShortId();
        AssociatedName fullId(context, Name(baseId.AsString() + id.AsString()));
        if(ids.contains(fullId)) {
            continue;
        }
        unique = true;
        result = fullId;
    }

    return result;
}
