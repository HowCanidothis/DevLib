#ifndef LOCALPROPERTYPALETTE_H
#define LOCALPROPERTYPALETTE_H

#include "translatormanager.h"

Q_DECLARE_METATYPE(SharedPointer<LocalPropertyColor>)
Q_DECLARE_METATYPE(SharedPointer<LocalPropertyDouble>)

DECLARE_GLOBAL_NAME(LOCALPROPERTY_PALETTE_COLOR)
DECLARE_GLOBAL_NAME(LOCALPROPERTY_PALETTE_LINE_WIDTH)
DECLARE_GLOBAL_NAME(LOCALPROPERTY_PALETTE_POINT_SIZE)
DECLARE_GLOBAL_NAME(LOCALPROPERTY_PALETTE_VISIBILITY)

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

class LocalPropertyPaletteObjectData : public QHash<Name, LocalPropertyPaletteData>
{
public:
    LocalPropertyPaletteObjectData()
    {}
};

class LocalPropertyPaletteObject
{
public:
    LocalPropertyPaletteObject();
    LocalPropertyPaletteObject(QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>>* dataTypes);

    bool IsEmpty() const { return m_data == nullptr ? true : m_data->isEmpty(); }

    SharedPointer<LocalPropertyBool> AsBool(const Name& key) const;
    SharedPointer<LocalPropertyDouble> AsDouble(const Name& key) const;
    SharedPointer<LocalPropertyColor> AsColor(const Name& key) const;

    SharedPointer<LocalPropertyBool> AsVisibility() const { return AsBool(LOCALPROPERTY_PALETTE_VISIBILITY); }
    SharedPointer<LocalPropertyDouble> AsLineWidth() const { return AsDouble(LOCALPROPERTY_PALETTE_LINE_WIDTH); }
    SharedPointer<LocalPropertyColor> AsPaletteColor() const { return AsColor(LOCALPROPERTY_PALETTE_COLOR); }
    SharedPointer<LocalPropertyDouble> AsPointSize() const { return AsDouble(LOCALPROPERTY_PALETTE_POINT_SIZE); }

    operator size_t() const { return (size_t)m_data.get(); }
    bool operator==(const LocalPropertyPaletteObject& another) const { return m_data.get() == another.m_data.get(); }

private:
    friend class LocalPropertyPalette;
    template<class T> friend struct Serializer;
    SharedPointer<LocalPropertyPaletteObjectData> m_data;
};

struct LocalPropertyPaletteBuilder
{
    LocalPropertyPaletteBuilder(){}
    LocalPropertyPaletteBuilder(bool /*defaultPalette*/);

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

    LocalPropertyPaletteObject& AddObjectIfNotExists(const Name& objectId, const std::function<void (LocalPropertyPaletteObject&)>& initializer)
    {
        const auto& found = FindObject(objectId);
        if(found.IsEmpty()) {
            LocalPropertyPaletteObject res(&m_storeTypes);
            initializer(res);
            return *m_data.insert(objectId, res);
        }
        return const_cast<LocalPropertyPaletteObject&>(found);
    }

    const LocalPropertyPaletteObject& FindObject(const Name& objectId) const;

private:
    friend struct Serializer<LocalPropertyPalette>;
    QHash<Name, LocalPropertyPaletteObject> m_data;
    QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>> m_storeTypes;
};
#endif // LOCALPROPERTYPALETTE_H
