#ifndef MODELSSERIALIZER_H
#define MODELSSERIALIZER_H

template<class T>
struct Serializer<TModelsTableWrapper<T>>
{
    using TypeName = TModelsTableWrapper<T>;

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& data)
    {
        buffer << buffer.Sect("Data", const_cast<TypeName&>(data).EditSilent());
    }

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            data.Change([&buffer, &data]{
                buffer << buffer.Sect("Data", data.EditSilent());
            });
        } else {
            buffer << buffer.Sect("Data", data.EditSilent());
        }
    }
};

template<class T>
struct SerializerXml<TModelsTableWrapper<T>>
{
    using Type = TModelsTableWrapper<T>;

    template<class Buffer>
    static void Read(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            object.Value.Change([&]{
                buffer << buffer.Sect(object.Name, object.Value.EditSilent());
            });
        } else {
            buffer << buffer.Sect(object.Name, object.Value.EditSilent());
        }
    }

    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer << buffer.Sect(object.Name, const_cast<Type&>(object.Value).EditSilent());
    }
};

#endif // MODELSSERIALIZER_H
