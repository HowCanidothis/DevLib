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

    DispatcherConnections ConnectFrom(const GtTextDrawableSettings& another)
    {
        DispatcherConnections result;
        result += Scale.ConnectFrom(another.Scale);
        result += Color.ConnectFrom(another.Color);
        result += BorderColor.ConnectFrom(another.BorderColor);
        result += BorderWidth.ConnectFrom(another.BorderWidth);
        result += Contrast.ConnectFrom(another.Contrast);
        result += UseDirectionCorrection.ConnectFrom(another.UseDirectionCorrection);
        return result;
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

struct GtTextScreenBasedGlyph : GtTextGlyphBase
{
    float Align;
    float TotalWidth;
    Vector2F Direction;
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
    const float& GetHeight() const { return m_height; }

private:
    qint32 m_borders[4];
    QSize m_size;
    QString m_fontFamily;
    qint32 m_fontSize;
    float m_height;
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

enum GtTextAlign
{
    GtTextAlign_HLeft = 0x1,
    GtTextAlign_HRight = 0x2,
    GtTextAlign_HCenter = GtTextAlign_HLeft | GtTextAlign_HRight,
    GtTextAlign_VTop = 0x4,
    GtTextAlign_VBottom = 0x8,
    GtTextAlign_VCenter = GtTextAlign_VTop | GtTextAlign_VBottom,
    GtTextAlign_Center = GtTextAlign_VCenter | GtTextAlign_HCenter,
    GtTextAlign_Directional = 0x10
};
DECL_FLAGS(GtTextAligns, GtTextAlign)

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

    enum Initializer
    {
        Text3D
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
            , Align(GtTextAlign_Center)
        {}

        GtTextAlign Align;
        Vector4F OffsetDirection; // w component is for distance

        TextInfo& SetAlign(GtTextAlign align) { Align = align; return *this; }
        TextInfo& SetOffsetDirection(const Vector4F& offsetDirection) { OffsetDirection = offsetDirection; return *this; }
    };

    GtTextDrawable(GtRenderer* renderer, const GtFontPtr& font, Initializer);
    GtTextDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram, const GtFontPtr& font);
    GtTextDrawable(GtRenderer* renderer, const GtFontPtr& font);

    void DisplayText(const QVector<TextInfo>& texts);

    GtTextDrawableSettings Settings;

private:
    GtMaterial m_material;
    GtMeshBufferPtr m_buffer;
    GtMeshBufferBuilder m_builder;
    GtFontPtr m_font;

    // GtDrawableBase interface
protected:
    void drawDepth(OpenGLFunctions* f) override;
    void draw(OpenGLFunctions* f) override;
    void onInitialize(OpenGLFunctions* f) override;
    void onDestroy(OpenGLFunctions* ) override {}
};

class GtTextScreenDrawable : public GtDrawableBase
{
    using Super = GtDrawableBase;
public:
#pragma pack(1)
    struct TextMeshStruct
    {
        Point2F Point;
        GtTextScreenBasedGlyph Glyph;
    };
#pragma pack()

    struct TextInfo
    {
        QString Text;
        Vector2F Position;
        Vector2F Direction;

        TextInfo() = default;
        TextInfo(const QString& text, const Vector2F& position, const Vector2F& direction)
            : Text(text)
            , Position(position)
            , Direction(direction)
            , Align(GtTextAlign_Center)
        {}

        GtTextAligns Align;

        TextInfo& SetAlign(GtTextAligns align) { Align = align; return *this; }
    };

    GtTextScreenDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram, const GtFontPtr& font);
    GtTextScreenDrawable(GtRenderer* renderer, const GtFontPtr& font);

    GtTextDrawableSettings Settings;

    void DisplayText(const QVector<TextInfo>& texts);

private:
    GtMaterial m_material;
    GtMeshBufferPtr m_buffer;
    GtMeshBufferBuilder m_builder;
    GtFontPtr m_font;

    // GtDrawableBase interface
protected:
    void drawDepth(OpenGLFunctions* f) override;
    void draw(OpenGLFunctions* f) override;
    void onInitialize(OpenGLFunctions* f) override;
    void onDestroy(OpenGLFunctions* ) override {}
};

#endif // GTTEXTDRAWABLE_H
