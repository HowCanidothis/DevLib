#ifndef LIBSERIALIZER_H
#define LIBSERIALIZER_H

#include "SharedModule/idgenerator.h"

#include "streambuffer.h"

template<>
struct Serializer<Name>
{
    typedef Name target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << data.AsString();
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        QString value;
        buffer << value;
        data.SetName(value);
    }
};

template<>
struct Serializer<Id::Id>
{
    using target_type = Id::Id;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << data.AsString();
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        QString value;
        buffer << value;
        data = Name(value);
    }
};

template<>
struct Serializer<Latin1Name>
{
    typedef Latin1Name target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << data.AsLatin1String();
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        std::string value;
        buffer << value;
        data.SetName(value);
    }
};

#endif // LIBSERIALIZER_H
