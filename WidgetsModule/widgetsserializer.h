#ifndef WIDGETSSERIALIZER_H
#define WIDGETSSERIALIZER_H

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
        buffer.OpenSection(object.Name);
        Serializer<Type>::Read(buffer, object.Value);
        buffer.CloseSection();
    }

    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer.OpenSection(object.Name);
        Serializer<Type>::Write(buffer, object.Value);
        buffer.CloseSection();
    }
};

#endif // WIDGETSSERIALIZER_H
