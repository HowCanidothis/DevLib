#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include "name.h"

namespace Id {

class Generator;

class Id : public Name
{
    using Super = Name;
public:
    Id(const Name& name = Name());
    ~Id();

    template<class T>
    void Attach(Generator* generator, const SharedPointer<T>& data);
    void Attach(Generator* generator);
    void Detach();
    bool IsAttached() const;
    Generator* GetGenerator() const;

    template<class T> SharedPointer<T> As() const;

    Id& operator=(const Name& another);

private:
    friend class Generator;
    void* getContext() const;

private:
    SharedPointer<struct IdData> m_data;
};

template<class T>
class WeakPtr : public std::weak_ptr<T> ATTACH_MEMORY_SPY_2(WeakPtr<T>)
{
    using Super = std::weak_ptr<T>;
public:
    using Super::Super;
};

class Generator
{
public:
    Generator(qint32 idSize = 10);
    ~Generator();
    Name BaseId;

    template<class T>
    Id CreateId(const SharedPointer<T>& data)
    {
        auto weakPtr = new WeakPtr<T>(data);
        return createId(weakPtr, [weakPtr] { delete weakPtr; });
    }

    Id GetId(const Name& id) const;

    bool IsComplexId() const { return m_idSize != 10; }

private:
    friend class Id;
    friend struct IdDataAttached;

    template<class T>
    void attachId(Id* id, const SharedPointer<T>& data)
    {
        auto weakPtr = new WeakPtr<T>(data);
        attach(id, weakPtr, [weakPtr] { delete weakPtr; });
    }

    Id createId(void* context, const FAction& deleter);
    void attach(Id* id, void* context = nullptr, const FAction& deleter = nullptr);
    void releaseId(const Name& id);
    Name generateComplexId();
    static Id createId(const Name& id, const SharedPointer<IdData>& iterator);

private:
    QHash<Name, WeakPtr<IdData>> m_registeredIds;
    qint32 m_idSize;
    std::function<std::pair<Name, SharedPointer<IdData>> (void* context, const FAction& deleter)> m_generator;
};

template<class T>
inline void Id::Attach(Generator* generator, const SharedPointer<T>& data)
{
    generator->attachId(this, data);
}

template<class T>
inline SharedPointer<T> Id::As() const
{
    const SharedPointer<T>& defaultValue = Default<SharedPointer<T>>::Value;
    auto* context = getContext();
    if(context == nullptr) {
        return defaultValue;
    }
    auto weakPtr = static_cast<WeakPtr<T>*>(context);
    if(weakPtr->expired()) {
        return defaultValue;
    }
    return static_cast<SharedPointer<T>>(std::shared_ptr(*weakPtr));
}

}



#endif // IDGENERATOR_H
