#ifndef Q_SERIALIZER_H
#define Q_SERIALIZER_H

#include <QString>

template<>
struct Serializer<QString>
{
    typedef QString target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        Serializer<uint32_t>::Write(buffer, static_cast<uint32_t>(data.size()));
        Serializer<PlainData>::Write(buffer, PlainData(data.constData(), data.size() * sizeof(target_type::value_type)));
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

#endif
