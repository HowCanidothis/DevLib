#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include "name.h"

class IdGenerator
{
public:
    class AssociatedName : public Name
    {
        using Super = Name;
    public:
        AssociatedName();
        AssociatedName(void* context, const Name& name = Name());

        template<class T> T* As() const { return static_cast<T*>(m_context); }

    private:
        void* m_context;
    };

    class Id
    {
    public:
        Id(IdGenerator* generator, const AssociatedName& id);
        Id(IdGenerator* generator, void* context);

        template<class T> T* As() const { return static_cast<T*>(getContext()); }
        bool IsNull() const;
        const QString& AsString() const;
        qint32 GetSize() const { return AsString().size(); }

        operator const AssociatedName&() const;

    private:
        void* getContext() const;

    private:
        SharedPointer<struct IdData> m_data;
    };

    IdGenerator(qint32 idSize = 10);

    void AddId(const AssociatedName& id);
    AssociatedName RegisterId(void* context, const AssociatedName& baseId = AssociatedName());
    void ReleaseId(const AssociatedName& id);

    Id RegisterIdObject(void* context, const AssociatedName& baseId = AssociatedName());

    static AssociatedName GenerateComplexId(void* context, const AssociatedName& baseId, const QSet<AssociatedName>& ids);

    bool IsComplexId() const { return m_idSize > 10; }

private:
    QSet<AssociatedName> m_registeredIds;
    qint32 m_idSize;
    std::function<AssociatedName (void* context, const AssociatedName& baseId)> m_generator;
};

#endif // IDGENERATOR_H
