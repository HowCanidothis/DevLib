#ifndef GTTEXTDRAWABLE_H
#define GTTEXTDRAWABLE_H

#include <optional>

#include <SharedGuiModule/internal.hpp>

#include "Objects/gtobjectbase.h"
#include "Objects/gtmaterial.h"
#include "gtmeshbase.h"

struct GtTextDrawableSettings
{
    LocalPropertyFloat Scale;
    LocalPropertyColor Color;
    LocalPropertyColor BorderColor;
    LocalPropertyFloat BorderWidth;
    LocalPropertyFloat Contrast;
    LocalPropertyBool UseDirectionCorrection;
    LocalPropertyBool Visible;

    GtTextDrawableSettings()
        : Scale(0.13)
        , Color(QColor(Qt::white))
        , BorderWidth(0.2)
        , Contrast(3.0)
        , UseDirectionCorrection(true)
        , Visible(true)
    {}

    void ConnectFrom(const GtTextDrawableSettings& another)
    {
        Scale.ConnectFrom(another.Scale);
        Color.ConnectFrom(another.Color);
        BorderColor.ConnectFrom(another.BorderColor);
        BorderWidth.ConnectFrom(another.BorderWidth);
        Contrast.ConnectFrom(another.Contrast);
        UseDirectionCorrection.ConnectFrom(another.UseDirectionCorrection);
    }
};

#pragma pack(1)
struct GtTextGlyphBase
{
    float X;
    float Y;
    float Width;
    float Height;
    float XOffset;
    float YOffset;
    float Index;
    float XAdvance;
};

struct GtTextGlyph : GtTextGlyphBase
{
    float TotalWidth;
    float Align;
    Vector3F Direction;
    Vector4F OffsetDirection;
};
#pragma pack()

class GtTextMap
{
public:
    void LoadFromFnt(const QString& fntFile);

    std::optional<GtTextGlyphBase> GetGlyph(qint32 unicode) const;

    QFontMetrics FontMetrics() const;

private:
    qint32 m_borders[4];
    QSize m_size;
    QString m_fontFamily;
    qint32 m_fontSize;
    QHash<qint32, GtTextGlyphBase> m_glyphs;
};

class GtFont
{
    friend class GtRenderer;
    GtFont(const Name& fontName, const QString& fntPath);
public:

    const Name& GetName() const { return m_name; }
    const GtTextMap& GetMap() const { return m_map; }

private:
    Name m_name;
    GtTextMap m_map;
};

class GtTextDrawable : public GtDrawableBase
{
    using Super = GtDrawableBase;
public:
#pragma pack(1)
    struct TextMeshStruct
    {
        Point3F Point;
        GtTextGlyph Glyph;
    };
#pragma pack()

    enum TextAlign
    {
        TextAlign_Center,
        TextAlign_Left,
        TextAlign_Right,
        TextAlign_Directional
    };

    struct TextInfo
    {
        QString Text;
        Vector3F Position;
        Vector3F Direction;

        TextInfo() = default;
        TextInfo(const QString& text, const Vector3F& position, const Vector3F& direction)
            : Text(text)
            , Position(position)
            , Direction(direction)
            , Align(TextAlign_Center)
        {}

        TextAlign Align;
        Vector4F OffsetDirection; // w component is for distance

        TextInfo& SetAlign(TextAlign align) { Align = align; return *this; }
        TextInfo& SetOffsetDirection(const Vector4F& offsetDirection) { OffsetDirection = offsetDirection; return *this; }
    };

    GtTextDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram, const GtFontPtr& font);
    GtTextDrawable(GtRenderer* renderer, const GtFontPtr& font);

    void DisplayText(const QVector<TextInfo>& texts);

    GtTextDrawableSettings Settings;

private:
    GtMaterial m_material;
    GtMeshBufferPtr m_buffer;
    GtMeshBufferBuilder m_builder;
    GtFontPtr m_font;
    QVector<SharedPointer<Property>> m_properties;

    // GtDrawableBase interface
protected:
    void drawDepth(OpenGLFunctions* f) override;
    void draw(OpenGLFunctions* f) override;
    void onInitialize(OpenGLFunctions* f) override;
    void onDestroy(OpenGLFunctions* ) override {}
};

#endif // GTTEXTDRAWABLE_H
