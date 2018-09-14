#ifndef STD_SERIALIZER_H
#define STD_SERIALIZER_H

#include <string>
#include <map>
#include <vector>
#include <deque>
#include <list>

#include "SharedModule/smartpointersadapters.h"
#include "SharedModule/array.h"
#include "SharedModule/stack.h"

struct PlainData
{
    PlainData(void* ptr, size_t count)
        : Ptr(reinterpret_cast<char*>(ptr))
        , Count(count)
    {}
    PlainData(const void* ptr, size_t count)
        : Ptr(reinterpret_cast<char*>(const_cast<void*>(ptr)))
        , Count(count)
    {}
    char* Ptr;
    size_t Count;
};

template<>
struct Serializer<PlainData>
{
    template<class Buffer>
    static void Write(Buffer& buffer, const PlainData& data)
    {
        buffer.GetStream().write(data.Ptr, data.Count);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, PlainData& data)
    {
        buffer.GetStream().read(data.Ptr, data.Count);
    }
};

template<>
struct Serializer<std::wstring>
{
    typedef std::wstring target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        Serializer<uint32_t>::Write(buffer, static_cast<uint32_t>(data.size()));
        Serializer<PlainData>::Write(buffer, PlainData(data.c_str(), data.size() * sizeof(target_type::value_type)));
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        uint32_t size;
        Serializer<uint32_t>::Read(buffer, size);
        data.resize(size);
        buffer << PlainData(data.data(), data.size() * sizeof(target_type::value_type));
    }
};

#define DECL_POINTER_SERIALIZER(type) \
template<> \
struct Serializer<type*> \
{ \
    typedef type* Pointer; \
    template<class Buffer> \
    static void Write(Buffer& buffer, const Pointer& pointer) \
    { \
        buffer << *pointer; \
    } \
    template<class Buffer> \
    static void Read(Buffer& buffer, Pointer& pointer) \
    { \
        pointer = new type(); \
        buffer << *pointer; \
    } \
};

#define DECL_SMART_POINTER_SERIALIZER(type) \
template<typename T> \
struct Serializer<type<T>> \
{ \
    typedef type<T> SmartPointer; \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SmartPointer& pointer) \
    { \
        buffer << *pointer; \
    } \
    template<class Buffer> \
    static void Read(Buffer& buffer, SmartPointer& pointer) \
    { \
        if(pointer == nullptr) { \
            pointer.reset(new T()); \
        } \
        buffer << *pointer; \
    } \
};

DECL_SMART_POINTER_SERIALIZER(std::shared_ptr)
DECL_SMART_POINTER_SERIALIZER(std::unique_ptr)
DECL_SMART_POINTER_SERIALIZER(SharedPointer)
DECL_SMART_POINTER_SERIALIZER(ScopedPointer)

#define DECL_PRIMITIVE_SERIALIZER(type) \
template<>\
struct Serializer<type> \
{ \
    template<class Buffer> \
    static void Write(Buffer& stream, type data) \
    { \
        stream.GetStream().write(reinterpret_cast<const char*>(&data), sizeof(type)); \
    } \
    template<class Buffer> \
    static void Read(Buffer& buffer, type& data) \
    { \
        buffer.GetStream().read(reinterpret_cast<char*>(&data), sizeof(type)); \
    } \
};

DECL_PRIMITIVE_SERIALIZER(bool)
DECL_PRIMITIVE_SERIALIZER(int32_t)
DECL_PRIMITIVE_SERIALIZER(int64_t)
DECL_PRIMITIVE_SERIALIZER(double_t)
DECL_PRIMITIVE_SERIALIZER(float_t)
DECL_PRIMITIVE_SERIALIZER(uint32_t)
DECL_PRIMITIVE_SERIALIZER(uint64_t)

#define DECL_STD_CONTAINER_SERIALIZER(ContainerType) \
template<typename T> \
struct Serializer<ContainerType<T>> \
{ \
    template<class Buffer> \
    static void Write(Buffer& buffer, const ContainerType<T>& container) \
    { \
        Serializer<int32_t>::Write(buffer, static_cast<int32_t>(container.size())); \
        for(auto& value : *const_cast<ContainerType<T>*>(&container)) { \
            buffer << value; \
        } \
    } \
    template<class Buffer> \
    static void Read(Buffer& buffer, ContainerType<T>& container) \
    { \
        int32_t size; \
        Serializer<int32_t>::Read(buffer, size); \
        container.resize(size); \
        for(auto& value : container) { \
            buffer << value; \
        } \
    } \
};

#define DECL_SHARED_MODULE_CONTAINER_SERIALIZER(ContainerType) \
template<typename T> \
struct Serializer<ContainerType<T>> \
{ \
    template<class Buffer> \
    static void Write(Buffer& buffer, const ContainerType<T>& container) \
    { \
        Serializer<int32_t>::Write(buffer, static_cast<int32_t>(container.Size())); \
        for(auto* value : *const_cast<ContainerType<T>*>(&container)) { \
            buffer << *value; \
        } \
    } \
    template<class Buffer> \
    static void Read(Buffer& buffer, ContainerType<T>& container) \
    { \
        int32_t size; \
        Serializer<int32_t>::Read(buffer, size); \
        container.Resize(size); \
        for(auto* value : container) { \
            buffer << *value; \
        } \
    } \
};

template<typename Key, typename Value>
struct Serializer<std::map<Key,Value>>
{
    typedef std::map<Key,Value> Container;
    template<class Buffer>
    static void Write(Buffer& buffer, const Container& map)
    {
        Serializer<int32_t>::Write(buffer, static_cast<int32_t>(map.size()));
        for(const auto& pair : map) {
            Serializer<Key>::Write(buffer, pair.first);
            Serializer<Value>::Write(buffer, pair.second);
        }
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Container& map)
    {
        int32_t size;
        Serializer<int32_t>::Read(buffer, size);
        while(size--) {
            Key key;
            Value value;
            Serializer<Key>::Read(buffer, key);
            Serializer<Value>::Read(buffer, value);
            map.try_emplace(key, value);
        }
    }
};

DECL_STD_CONTAINER_SERIALIZER(std::vector)
DECL_STD_CONTAINER_SERIALIZER(std::deque)
DECL_STD_CONTAINER_SERIALIZER(std::list)

DECL_SHARED_MODULE_CONTAINER_SERIALIZER(StackPointers)
DECL_SHARED_MODULE_CONTAINER_SERIALIZER(ArrayPointers)

template<typename First, typename Second>
struct Serializer<std::pair<First, Second>>
{
    typedef std::pair<First, Second> Pair;
    template<class Buffer>
    static void Write(Buffer& buffer, const Pair& pair)
    {
        buffer << pair.first;
        buffer << pair.second;
    }
    template<class Buffer>
    static void Read(Buffer& buffer, Pair& pair)
    {
        buffer << pair.first;
        buffer << pair.second;
    }
};

#endif
