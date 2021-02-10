#ifndef PROPERTIESSERIALIZER_H
#define PROPERTIESSERIALIZER_H

#include "localproperty.h"

template<typename T>
struct Serializer<LocalProperty<T>>
{
    typedef LocalProperty<T> target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << data.m_value;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            T value;
            buffer << value;
            data = value;
        } else {
            buffer << data.m_value;
        }
    }
};

template<typename T>
struct Serializer<LocalPropertySequentialEnum<T>>
{
    typedef LocalPropertySequentialEnum<T> target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << data.m_value;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            qint32 value;
            buffer << value;
            data = value;
        } else {
            buffer << data.m_value;
        }
    }
};

template<typename T>
struct Serializer<LocalPropertyLimitedDecimal<T>>
{
    typedef LocalPropertyLimitedDecimal<T> target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_MinMaxProperties)) {
            buffer << data.m_max;
            buffer << data.m_min;
        }
        buffer << data.m_value;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_MinMaxProperties)) {
            buffer << data.m_max;
            buffer << data.m_min;
        }
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            T value;
            buffer << value;
            data = value;
        } else {
            buffer << data.m_value;
        }        
    }
};

template<typename T>
struct Serializer<LocalPropertyVector<T>>
{
    using TypeName = LocalPropertyVector<T>;

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& data)
    {
        qint32 size = data.Size();
        buffer << size;
        for(const T& value : data) {
            buffer << value;
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& data)
    {
        qint32 size;
        buffer << size;
        auto& vector = data.EditSilent();
        vector.resize(size);
        for(T& value : vector) {
            buffer << value;
        }
        vector.Invoke();
    }
};

#endif // PROPERTIESSERIALIZER_H
