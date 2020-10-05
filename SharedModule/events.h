#ifndef EVENTS_H
#define EVENTS_H

#include <Qt>

template<class Enum>
class EventBase
{
public:
    EventBase()
        : m_accepted(false)
    {}

    virtual Enum GetType() const = 0;

    template<class T> T* As() { Q_ASSERT(false); return nullptr; }

    void Accept() { m_accepted = true; }
    bool IsAccepted() const { return m_accepted; }

private:
    bool m_accepted;
};

#define SHARED_REGISTER_OBJECT_EVENT(eventClassName, className) \
template<> \
inline className* eventClassName::As() \
{ \
    Q_ASSERT(className::GetEventType() == GetType()); return reinterpret_cast<className*>(this); \
}

template<class Enum, class Parent, Enum Type, typename T>
class EventValue : public Parent
{
    T m_value;
public:
    static Enum GetEventType() { return Type; }

    EventValue(const T& value)
        : m_value(value)
    {}

    const T& GetValue() const { return m_value; }
    Enum GetType() const override { return Type; }
};

template<class Enum, class Parent, Enum Type>
class EventVoid : public Parent
{
public:
    static Enum GetEventType() { return Type; }

    Enum GetType() const override { return Type; }
};

#define SHARED_DECLARE_EVENT_FOR_ENUM(prefix, enumeration) \
class prefix##EventBase : public EventBase<enumeration> \
{ \
public: \
    template<class T> T* As() { Q_ASSERT(false); return nullptr; } \
}; \
template<enumeration Type, class T> \
class prefix##EventValue : public EventValue<enumeration, prefix##EventBase, Type, T> \
{ \
    using Super = EventValue<enumeration, prefix##EventBase, Type, T>; \
public: \
    using Super::Super; \
}; \
template<enumeration Type> \
class prefix##EventVoid : public EventVoid<enumeration, prefix##EventBase, Type> \
{ \
    using Super = EventVoid<enumeration, prefix##EventBase, Type>; \
public: \
    using Super::Super; \
};

/* Usage Sample. Creates Foo prefixed events with FooEventType enumeration. For event dispatcher is supposed to use CommonDispatcher<FooEventBase*>
 *  #define FOO_REGISTER_OBJECT_EVENT(className) \
    SHARED_REGISTER_OBJECT_EVENT(FooEventBase, className)

    SHARED_DECLARE_EVENT_FOR_ENUM(Foo, FooEventType)
 *
*/
#endif // EVENTS_H
