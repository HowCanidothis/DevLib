#ifndef LOCALPROPERTYPALETTE_H
#define LOCALPROPERTYPALETTE_H


Q_DECLARE_METATYPE(SharedPointer<LocalPropertyColor>)
Q_DECLARE_METATYPE(SharedPointer<LocalPropertyDouble>)

struct LocalPropertyPaletteDataData
{
    enum SupportedType
    {
        Unknown,
        Boolean,
        Color,
        Double
    };

    LocalPropertyPaletteDataData(const QVariant& value, SupportedType type);
    SharedPointer<LocalPropertyBool> AsBool() const;
    SharedPointer<LocalPropertyDouble> AsDouble() const;
    SharedPointer<LocalPropertyColor> AsColor() const;

    SupportedType GetType() const { return m_type; }

    void FromString(const QString& value)
    {
        m_fromStringHandler(value);
    }

    QString ToString() const
    {
        return m_toStringHandler();
    }

    friend QDebug operator <<(QDebug debug, const LocalPropertyPaletteDataData& data)
    {
        debug << data.m_type;
        return debug.maybeSpace();
    }

private:
    SupportedType m_type;
    QVariant m_value;
    std::function<void (const QString&)> m_fromStringHandler;
    std::function<QString ()> m_toStringHandler;
};

class LocalPropertyPaletteData
{
public:
    LocalPropertyPaletteData();
    LocalPropertyPaletteData(const QVariant& value, LocalPropertyPaletteDataData::SupportedType type);

    const SharedPointer<LocalPropertyPaletteDataData>& GetData() const { return m_data; }

    friend QDebug operator<<(QDebug debug, const LocalPropertyPaletteData& data)
    {
        if(data.m_data == nullptr) {
            debug << "LocalPropertyPaletteData::NoValue";
        } else {
            debug << data.m_data->GetType();
        }
        return debug.maybeSpace();
    }

private:
    SharedPointer<LocalPropertyPaletteDataData> m_data;
};

class LocalPropertyPaletteObject : protected QHash<Name, LocalPropertyPaletteData>
{
public:
    LocalPropertyPaletteObject();
    LocalPropertyPaletteObject(QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>>* dataTypes);

    bool IsEmpty() const { return isEmpty(); }

    SharedPointer<LocalPropertyBool> AsBool(const Name& key) const;
    SharedPointer<LocalPropertyDouble> AsDouble(const Name& key) const;
    SharedPointer<LocalPropertyColor> AsColor(const Name& key) const;

private:
    friend class LocalPropertyPalette;
    template<class T> friend struct Serializer;
    QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>>* m_dataTypes;
};

struct LocalPropertyPaletteBuilder
{
    LocalPropertyPaletteBuilder(){}

    LocalPropertyPaletteBuilder& AddDouble(const Name& key, double defaultValue);
    LocalPropertyPaletteBuilder& AddBool(const Name& key, bool defaultValue);
    LocalPropertyPaletteBuilder& AddColor(const Name& key, const QColor& defaultValue);

    const QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>>& GetResult() const { return m_result; }

private:
    QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>> m_result;
};

class LocalPropertyPalette
{
public:
    LocalPropertyPalette(const LocalPropertyPaletteBuilder& builder)
        : m_storeTypes(builder.GetResult())
    {}

    LocalPropertyPaletteObject& AddObject(const Name& objectId)
    {
        return *m_data.insert(objectId, LocalPropertyPaletteObject(&m_storeTypes));
    }

    const LocalPropertyPaletteObject& FindObject(const Name& objectId) const;

private:
    friend struct Serializer<LocalPropertyPalette>;
    QHash<Name, LocalPropertyPaletteObject> m_data;
    QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>> m_storeTypes;
};
#endif // LOCALPROPERTYPALETTE_H
