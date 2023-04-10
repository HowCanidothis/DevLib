#include "localpropertypalette.h"

LocalPropertyPaletteDataData::LocalPropertyPaletteDataData(const QVariant& value, SupportedType type)
    : m_type(type)
{
    switch(m_type)
    {
    case Boolean: m_value = QVariant::fromValue(::make_shared<LocalPropertyBool>(value.toBool()));
        m_fromStringHandler = [this](const QString& value){ *AsBool() = TextConverter<bool>::FromText(value); };
        m_toStringHandler = [this]{ return TextConverter<bool>::ToText(*AsBool(), TextConverterContext()); };
        break;
    case Color: m_value = QVariant::fromValue(::make_shared<LocalPropertyColor>(value.value<QColor>()));
        m_fromStringHandler = [this](const QString& value){ *AsColor() = TextConverter<QColor>::FromText(value); };
        m_toStringHandler = [this]{ return TextConverter<QColor>::ToText(*AsColor(), TextConverterContext()); };
        break;
    case Double: m_value = QVariant::fromValue(::make_shared<LocalPropertyDouble>(value.value<double>()));
        m_fromStringHandler = [this](const QString& value){ *AsDouble() = TextConverter<double>::FromText(value); };
        m_toStringHandler = [this]{ return TextConverter<double>::ToText(*AsDouble(), TextConverterContext()); };
        break;
    default:
        m_fromStringHandler = [](const QString&){};
        m_toStringHandler = TR_NONE;
        break;
    }
}

SharedPointer<LocalPropertyBool> LocalPropertyPaletteDataData::AsBool() const
{
    Q_ASSERT(m_type == Boolean);
    return m_value.value<SharedPointer<LocalPropertyBool>>();
}

SharedPointer<LocalPropertyDouble> LocalPropertyPaletteDataData::AsDouble() const
{
    Q_ASSERT(m_type == Double);
    return m_value.value<SharedPointer<LocalPropertyDouble>>();
}

SharedPointer<LocalPropertyColor> LocalPropertyPaletteDataData::AsColor() const
{
    Q_ASSERT(m_type == Color);
    return m_value.value<SharedPointer<LocalPropertyColor>>();
}

LocalPropertyPaletteData::LocalPropertyPaletteData()
{}
LocalPropertyPaletteData::LocalPropertyPaletteData(const QVariant& value, LocalPropertyPaletteDataData::SupportedType type)
    : m_data(make_shared<LocalPropertyPaletteDataData>(value, type))
{}

LocalPropertyPaletteObject::LocalPropertyPaletteObject()
{}
LocalPropertyPaletteObject::LocalPropertyPaletteObject(QHash<Name, std::pair<LocalPropertyPaletteDataData::SupportedType, QVariant>>* dataTypes)
    : m_data(::make_shared<LocalPropertyPaletteObjectData>())
{
    for(auto it(dataTypes->cbegin()), e(dataTypes->cend()); it != e; ++it) {
        m_data->insert(it.key(), LocalPropertyPaletteData(it.value().second, it.value().first));
    }
}

SharedPointer<LocalPropertyBool> LocalPropertyPaletteObject::AsBool(const Name& key) const
{
    return m_data->value(key).GetData()->AsBool();
}

SharedPointer<LocalPropertyDouble> LocalPropertyPaletteObject::AsDouble(const Name& key) const
{
    return m_data->value(key).GetData()->AsDouble();
}

SharedPointer<LocalPropertyColor> LocalPropertyPaletteObject::AsColor(const Name& key) const
{
    return m_data->value(key).GetData()->AsColor();
}

const LocalPropertyPaletteObject& LocalPropertyPalette::FindObject(const Name& objectId) const
{
    static LocalPropertyPaletteObject defaultValue;
    auto foundIt = m_data.find(objectId);
    if(foundIt != m_data.end()) {
        return foundIt.value();
    }
    return defaultValue;
}

LocalPropertyPaletteBuilder::LocalPropertyPaletteBuilder(bool)
{
    AddColor(LOCALPROPERTY_PALETTE_COLOR, QColor()).
    AddBool(LOCALPROPERTY_PALETTE_VISIBILITY, true).
    AddDouble(LOCALPROPERTY_PALETTE_LINE_WIDTH, 4.0).
    AddDouble(LOCALPROPERTY_PALETTE_POINT_SIZE, 4.0);
}

LocalPropertyPaletteBuilder& LocalPropertyPaletteBuilder::AddDouble(const Name& key, double defaultValue)
{
    m_result.insert(key, { LocalPropertyPaletteDataData::Double, defaultValue });
    return *this;
}
LocalPropertyPaletteBuilder& LocalPropertyPaletteBuilder::AddBool(const Name& key, bool defaultValue)
{
    m_result.insert(key, { LocalPropertyPaletteDataData::Boolean, defaultValue });
    return *this;
}
LocalPropertyPaletteBuilder& LocalPropertyPaletteBuilder::AddColor(const Name& key, const QColor& defaultValue)
{
    m_result.insert(key, { LocalPropertyPaletteDataData::Color, defaultValue });
    return *this;
}

IMPLEMENT_GLOBAL_NAME(LOCALPROPERTY_PALETTE_COLOR,Color)
IMPLEMENT_GLOBAL_NAME(LOCALPROPERTY_PALETTE_LINE_WIDTH,LineWidth)
IMPLEMENT_GLOBAL_NAME(LOCALPROPERTY_PALETTE_POINT_SIZE,PointSize)
IMPLEMENT_GLOBAL_NAME(LOCALPROPERTY_PALETTE_VISIBILITY,Visibility)
