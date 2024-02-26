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

template<>
struct Serializer<StateProperty>
{
    typedef StateProperty target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << data.m_value;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            bool value;
            buffer << value;
            data.SetState(value);
        } else {
            buffer << data.m_value;
        }
    }
};

template<>
struct Serializer<LocalPropertyFilePath>
{
    typedef LocalPropertyFilePath target_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& data)
    {
        buffer << data.m_value;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            QString value;
            buffer << value;
            data = value;
        } else {
            buffer << data.m_value;
        }
    }
};

template<class T>
struct Serializer<LocalPropertyOptional<T>>
{
    typedef LocalPropertyOptional<T> target_type;
    using value_type = typename T::value_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& constData)
    {
        auto& data = const_cast<target_type&>(constData);
        buffer << buffer.Attr("IsValid", data.IsValid.EditSilent());
        if(data.IsValid) {
            buffer << buffer.Attr("Value", data.Value);
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        bool valid;
        buffer << buffer.Attr("IsValid", valid);
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            if(valid) {
                buffer << buffer.Attr("Value", data.Value);
            }
            data.IsValid = valid;
        } else {
            if(valid) {
                buffer << buffer.Attr("Value", data.Value.EditSilent());
            }
            data.IsValid.EditSilent() = valid;
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

template<class T>
struct Serializer<LocalPropertySet<T>>
{
    using TypeName = LocalPropertySet<T>;

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& object)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            QSet<T> data;
            buffer << buffer.Sect("Data", data);
            object.EditSilent() = data;
            object.Invoke();
            return;
        }
        buffer << buffer.Sect("Data", object.EditSilent());
    }

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& object)
    {
        buffer << buffer.Sect("Data", const_cast<TypeName&>(object).EditSilent());
    }
};

DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER(LocalPropertySet)

#ifdef SHARED_LIB_ADD_UI

#include "Ui/localpropertypalette.h"

template<>
struct Serializer<LocalPropertyPalette>
{
    using TypeName = LocalPropertyPalette;

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& object)
    {
        buffer.OpenSection("PropertiesPalette");
        qint32 count = 0;
        buffer << buffer.Sect("Count", count);

        auto& data = object.m_data;
        while(count--) {
            Name name; qint32 count2 = 0;
            buffer << buffer.Sect("Name", name);
            buffer << buffer.Sect("Count", count2);
            auto foundIt = data.find(name);
            std::function<void (const Name&, const QString&)> handler;
            if(foundIt != data.end()) {
                handler = [&foundIt](const Name& name, const QString& value){
                    auto foundIt2 = foundIt.value().m_data->find(name);
                    if(foundIt2 != foundIt.value().m_data->end()) {
                        foundIt2.value().GetData()->FromString(value);
                    }
                };
            } else {
                handler = [](const Name&, const QString&){};
            }

            while(count2--) {
                QString value;
                buffer << buffer.Sect("Name", name);
                buffer << buffer.Sect("Value", value);
                handler(name, value);
            }
        }
        buffer.CloseSection();
    }

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& object)
    {
        buffer.OpenSection("PropertiesPalette");
        auto& data = object.m_data;
        qint32 count = data.size();
        buffer << buffer.Sect("Count", count);
        for(auto it(data.begin()), e(data.end()); it != e; ++it) {
            buffer.OpenSection("Object");
            const Name& key = it.key();
            buffer << buffer.Sect("Name", const_cast<Name&>(key));
            count = it.value().m_data->size();
            buffer << buffer.Sect("Count", count);
            for(auto objectIt(it.value().m_data->begin()), objectE(it.value().m_data->end()); objectIt != objectE; ++objectIt) {
                const Name& okey = objectIt.key();
                buffer << buffer.Sect("Name", const_cast<Name&>(okey));
                QString toString = objectIt->GetData()->ToString();
                buffer << buffer.Sect("Value", toString);
            }
            buffer.CloseSection();
        }
        buffer.CloseSection();
    }
};

DECLARE_SERIALIZER_XML_TO_SERIALIZER(LocalPropertyPalette)
#endif
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

#define SERIALIZER_XML_DECLARE_PROPERTY_TYPE(PropertyType) \
template<class T> \
struct SerializerXml<PropertyType<T>> \
{ \
    using Type = PropertyType<T>; \
    template<class Buffer> \
    static void Read(Buffer& buffer, SerializerXmlObject<Type>& object) \
    { \
        T value; \
        buffer << object.Mutate(value); \
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) { \
            object.Value = value; \
        } else { \
            object.Value.EditSilent() = value; \
        } \
    } \
    template<class Buffer> \
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object) \
    { \
        const auto& value = object.Value.Native(); \
        buffer << object.Mutate(const_cast<T&>(value)); \
    } \
};

template<class T>
struct SerializerXml<LocalPropertySequentialEnum<T>>
{
    using Type = LocalPropertySequentialEnum<T>;
    template<class Buffer>
    static void Read(Buffer& buffer, SerializerXmlObject<Type>& object)
    {
        qint32 value;
        buffer << object.Mutate(value);
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            object.Value = (T)value;
        } else {
            object.Value.m_value = value;
        }
    }
    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer << object.Mutate(object.Value.m_value);
    }
};

template<>
struct SerializerXml<StateProperty>
{
    using Type = StateProperty;
    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& data)
    {
        buffer << data.Mutate(data.Value.m_value);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, SerializerXmlObject<Type>& data)
    {
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            bool value;
            buffer << buffer.Sect(data.Name, value);
            data.Value.SetState(value);
        } else {
            buffer << data.Mutate(data.Value.m_value);
        }
    }
};

template<>
struct SerializerXml<LocalPropertyFilePath>
{
    using Type = LocalPropertyFilePath;
    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& data)
    {
        buffer << data.Mutate(data.Value.m_value);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, SerializerXmlObject<Type>& data)
    {
        QString value;
        buffer << data.Mutate(value);

        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            data.Value = value;
        } else {
            data.Value.m_value = value;
        }
    }
};

template<class T>
struct Serializer<StateParameterProperty<T>>
{
    typedef StateParameterProperty<T> target_type;
    using value_type = typename T::value_type;
    template<class Buffer>
    static void Write(Buffer& buffer, const target_type& constData)
    {
        auto& data = const_cast<target_type&>(constData);
        buffer << data.InputValue;
    }

    template<class Buffer>
    static void Read(Buffer& buffer, target_type& data)
    {
        buffer << data.InputValue;
        if(!buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            data.m_immutableValue.SetSilent(data.InputValue);
        }
    }
};

template<class T>
struct SerializerXml<StateParameterProperty<T>>
{
    using Type = StateParameterProperty<T>;
    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        buffer << object.Mutate(object.Value.InputValue);
    }

    template<class Buffer>
    static void Read(Buffer& buffer, SerializerXmlObject<Type>& object)
    {
        buffer << object.Mutate(object.Value.InputValue);
        auto& data = object.Value;
        if(!buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            data.m_immutableValue.SetSilent(data.InputValue);
        }
    }
};

template<>
struct Serializer<LocalPropertyDate>
{
    typedef LocalPropertyDate target_type;
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
            QDate value;
            buffer << value;
            data = value;
        } else {
            buffer << data.m_value;
        }
    }
};
template<>
struct SerializerXml<LocalPropertyDate>
{
    using Type = LocalPropertyDate;
    template<class Buffer>
    static void Read(Buffer& buffer, SerializerXmlObject<Type>& object)
    {
        QDate value;
        buffer << object.Mutate(value);
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            object.Value = value;
        } else {
            object.Value.EditSilent() = value;
        }
    }
    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        const auto& value = object.Value.Native();
        buffer << object.Mutate(const_cast<QDate&>(value));
    }
};

template<>
struct Serializer<LocalPropertyDateTime>
{
    typedef LocalPropertyDateTime target_type;
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
            QDateTime value;
            buffer << value;
            data = value;
        } else {
            buffer << data.m_value;
        }        
    }
};
template<>
struct SerializerXml<LocalPropertyDateTime>
{
    using Type = LocalPropertyDateTime;
    template<class Buffer>
    static void Read(Buffer& buffer, SerializerXmlObject<Type>& object)
    {
        QDateTime value;
        buffer << object.Mutate(value);
        if(buffer.GetSerializationMode().TestFlag(SerializationMode_InvokeProperties)) {
            object.Value = value;
        } else {
            object.Value.EditSilent() = value;
        }
    }
    template<class Buffer>
    static void Write(Buffer& buffer, const SerializerXmlObject<Type>& object)
    {
        const auto& value = object.Value.Native();
        buffer << object.Mutate(const_cast<QDateTime&>(value));
    }
};

SERIALIZER_XML_DECLARE_PROPERTY_TYPE(LocalProperty)
SERIALIZER_XML_DECLARE_PROPERTY_TYPE(LocalPropertyLimitedDecimal)
DECLARE_SERIALIZER_TYPE_ALIAS(LocalProperty<bool>, LocalPropertyBool)
DECLARE_SERIALIZER_XML_TYPE_ALIAS(LocalProperty<bool>, LocalPropertyBool)
DECLARE_SERIALIZER_XML_CONTAINER_TO_SERIALIZER(LocalPropertyOptional)

#endif // PROPERTIESSERIALIZER_H
