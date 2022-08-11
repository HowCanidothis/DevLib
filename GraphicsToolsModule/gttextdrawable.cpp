#include "gttextdrawable.h"

#include "Objects/gtmaterialparametermatrix.h"
#include "Objects/gtmaterialparametervector3f.h"
#include "Objects/gtmaterialparametertexture.h"
#include "gttexture2D.h"
#include "gtrenderer.h"

void GtTextMap::LoadFromFnt(const QString& fntFile)
{
    m_glyphs.clear();
    QFile file(fntFile);
    if(file.open(QFile::ReadOnly)) {
        auto contents = file.readAll();
        QRegExp regexp(R"((\w+)=([^\s]+))");
        qint32 pos = 0;
        m_height = 0;

        qint32 id;
        GtTextGlyphBase currentGlyph;
        QHash<Name, std::function<void (const QString& ref)>> factory=
        {
        { Name("padding"), [this](const QString& ref) {
            qint32 i=0;
            for(const auto& border : ref.split(",")) {
                m_borders[i] = border.toInt();
            }
        } },
        { Name("scaleW"), [this](const QString& ref) { m_size.rwidth() = ref.toInt(); } },
        { Name("scaleH"), [this](const QString& ref) { m_size.rheight() = ref.toInt(); } },

        { Name("id"), [&id](const QString& ref) { id = ref.toInt(); } },
        { Name("x"), [&currentGlyph](const QString& ref) { currentGlyph.X = ref.toFloat(); } },
        { Name("y"), [&currentGlyph](const QString& ref) { currentGlyph.Y = ref.toFloat(); } },
        { Name("width"), [&currentGlyph](const QString& ref) { currentGlyph.Width = ref.toFloat(); } },
        { Name("height"), [&currentGlyph, this](const QString& ref) {
            currentGlyph.Height = ref.toFloat();
            if(currentGlyph.Height > m_height) {
                m_height = currentGlyph.Height;
            }
        } },
        { Name("xoffset"), [&currentGlyph](const QString& ref) { currentGlyph.XOffset = ref.toFloat(); } },
        { Name("yoffset"), [&currentGlyph](const QString& ref) { currentGlyph.YOffset = ref.toFloat(); } },
        { Name("xadvance"), [&currentGlyph](const QString& ref) { currentGlyph.XAdvance = ref.toFloat(); } },
        { Name("face"), [this](const QString& ref) { m_fontFamily = ref; } },
        { Name("size"), [this](const QString& ref) { m_fontSize = ref.toInt(); } },
        { Name("chnl"), [this, &id, &currentGlyph](const QString&) {
            m_glyphs.insert(id, currentGlyph);
        } },
    };

        while((pos = regexp.indexIn(contents, pos)) != -1) {
            auto foundIt = factory.find(Name(regexp.cap(1)));
            if(foundIt != factory.end()) {
                foundIt.value()(regexp.cap(2));
            }
            pos += regexp.matchedLength();
        }
    }
}

std::optional<GtTextGlyphBase> GtTextMap::GetGlyph(qint32 unicode) const
{
    auto foundIt = m_glyphs.find(unicode);
    if(foundIt == m_glyphs.end()) {
        return std::nullopt;
    }
    return std::make_optional(foundIt.value());
}

QFontMetrics GtTextMap::FontMetrics() const
{
    QFont font(m_fontFamily);
    font.setPointSize(m_fontSize);
    return QFontMetrics(font);
}

GtTextDrawable::GtTextDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram, const GtFontPtr& font)
    : Super(renderer)
    , m_material(GL_POINTS, shaderProgram)
    , m_buffer(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Custom, QOpenGLBuffer::StaticDraw))
    , m_font(font)
{
    m_material.AddMesh(::make_shared<GtMesh>(m_buffer));
    m_material.AddParameter(::make_shared<GtMaterialParameterMatrix>("MVP", GtNames::mvp));
    m_material.AddParameter(::make_shared<GtMaterialParameterVector2F>("SCREEN_SIZE", GtNames::screenSize), false);
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("MODEL_MATRIX", &Transform.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("ENABLE_DIR_CORRECTION", &Settings.UseDirectionCorrection.Native()), false);
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("BORDER_WIDTH", &Settings.BorderWidth.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("CONTRAST", &Settings.Contrast.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("TEXT_SCALE", &Settings.Scale.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("COLOR", &Settings.Color.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("BORDER_COLOR", &Settings.BorderColor.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("TEXT_HEIGHT", &font->GetMap().GetHeight()));
    m_material.AddParameter(::make_shared<GtMaterialParameterTexture>("TEXTURE", font->GetName()));

    m_builder.AddComponent<float>(3).
              AddComponent<float>(4).
              AddComponent<float>(2).
              AddComponent<float>(4).
              AddComponent<float>(3).
              AddComponent<float>(4);

    Settings.Visible.Subscribe([this]{
        Update([this]{
            m_material.SetVisible(Settings.Visible);
        });
    });
    Transform.SetSetterHandler(CreateThreadHandler());
}

GtTextDrawable::GtTextDrawable(GtRenderer* renderer, const GtFontPtr& font)
    : GtTextDrawable(renderer, renderer->GetShaderProgram("DefaultTextShaderProgram"), font)
{

}

GtTextDrawable::GtTextDrawable(GtRenderer* renderer, const GtFontPtr& font, Initializer)
    : GtTextDrawable(renderer, renderer->GetShaderProgram("DefaultText3DShaderProgram"), font)
{

}

void GtTextDrawable::DisplayText(const QVector<GtTextDrawable::TextInfo>& texts)
{
    Update([texts,this](OpenGLFunctions* f){
        QVector<TextMeshStruct> mesh;
        for(const auto& textInfo : texts) {
            TextMeshStruct currentMeshVertex;
            currentMeshVertex.Glyph.XAdvance = 0.f;
            currentMeshVertex.Point = textInfo.Position;
            currentMeshVertex.Glyph.Direction = textInfo.Direction;
            currentMeshVertex.Glyph.Align = textInfo.Align;
            currentMeshVertex.Glyph.OffsetDirection = textInfo.OffsetDirection;
            qint32 index = 0;
            auto meshSize = mesh.size();
            for(const auto& glyph : textInfo.Text) {
                auto glyphInfo = m_font->GetMap().GetGlyph(glyph.unicode());
                if(glyphInfo.has_value()) {
                    auto oldAdvance = currentMeshVertex.Glyph.XAdvance;
                    reinterpret_cast<GtTextGlyphBase&>(currentMeshVertex.Glyph) = glyphInfo.value();
                    currentMeshVertex.Glyph.XAdvance = oldAdvance;
                    currentMeshVertex.Glyph.Index = index++;
                    mesh.append(currentMeshVertex);
                    currentMeshVertex.Glyph.XAdvance += glyphInfo->XAdvance;
                }
            }
            auto totalWidth = currentMeshVertex.Glyph.XAdvance + 10.f * (mesh.size() - meshSize);
            for(auto& glyph : adapters::range(mesh.begin() + meshSize, mesh.end())) {
                glyph.Glyph.TotalWidth = totalWidth;
            }
        }
        m_buffer->UpdateVertexArray(mesh, m_builder);
        m_buffer->UpdateVao(f);
    });
}

void GtTextDrawable::draw(OpenGLFunctions* f)
{
    m_material.Draw(f);
    f->glPointSize(10.f); // TODO. ?
}

void GtTextDrawable::drawDepth(OpenGLFunctions* )
{
}

void GtTextDrawable::onInitialize(OpenGLFunctions* f)
{
    m_buffer->Initialize(f);
}

GtFont::GtFont(const Name& fontName, const QString& fntPath)
    : m_name(fontName)
{
    m_map.LoadFromFnt(fntPath);
}

GtTextScreenDrawable::GtTextScreenDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram, const GtFontPtr& font)
    : Super(renderer)
    , m_material(GL_POINTS, shaderProgram)
    , m_buffer(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Custom, QOpenGLBuffer::StaticDraw))
    , m_font(font)
{
    m_material.AddMesh(::make_shared<GtMesh>(m_buffer));
    m_material.AddParameter(::make_shared<GtMaterialParameterVector2F>("SCREEN_SIZE", GtNames::screenSize));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("BORDER_WIDTH", &Settings.BorderWidth.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("CONTRAST", &Settings.Contrast.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("TEXT_SCALE", &Settings.Scale.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("COLOR", &Settings.Color.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("BORDER_COLOR", &Settings.BorderColor.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("TEXT_HEIGHT", &font->GetMap().GetHeight()));
    m_material.AddParameter(::make_shared<GtMaterialParameterTexture>("TEXTURE", font->GetName()));

    m_builder.AddComponent<float>(2).
              AddComponent<float>(4).
              AddComponent<float>(2).
              AddComponent<float>(4).
              AddComponent<float>(2).
              AddComponent<float>(3);

    Settings.Visible.Subscribe([this]{
        Update([this]{
            m_material.SetVisible(Settings.Visible);
        });
    });
}

GtTextScreenDrawable::GtTextScreenDrawable(GtRenderer* renderer, const GtFontPtr& font)
    : GtTextScreenDrawable(renderer, renderer->GetShaderProgram("DefaultScreenTextShaderProgram"), font)
{

}

void GtTextScreenDrawable::DisplayText(const QVector<TextInfo>& texts)
{
    Update([texts,this](OpenGLFunctions* f){
        Color3F settingsColor = Color3FCreate(Settings.Color);
        QVector<TextMeshStruct> mesh;
        for(const auto& textInfo : texts) {
            TextMeshStruct currentMeshVertex;
            currentMeshVertex.Glyph.Color = textInfo.m_colorHandler(settingsColor);
            currentMeshVertex.Glyph.XAdvance = 0.f;
            currentMeshVertex.Point = textInfo.Position;
            currentMeshVertex.Glyph.Direction = textInfo.Direction;
            currentMeshVertex.Glyph.Align = textInfo.Align;
            qint32 index = 0;
            auto meshSize = mesh.size();
            for(const auto& glyph : textInfo.Text) {
                auto glyphInfo = m_font->GetMap().GetGlyph(glyph.unicode());
                if(glyphInfo.has_value()) {
                    auto oldAdvance = currentMeshVertex.Glyph.XAdvance;
                    reinterpret_cast<GtTextGlyphBase&>(currentMeshVertex.Glyph) = glyphInfo.value();
                    currentMeshVertex.Glyph.XAdvance = oldAdvance;
                    currentMeshVertex.Glyph.Index = index++;
                    mesh.append(currentMeshVertex);
                    currentMeshVertex.Glyph.XAdvance += glyphInfo->XAdvance;
                }
            }
            auto totalWidth = currentMeshVertex.Glyph.XAdvance + 10.f * (mesh.size() - meshSize);
            for(auto& glyph : adapters::range(mesh.begin() + meshSize, mesh.end())) {
                glyph.Glyph.TotalWidth = totalWidth;
            }
        }
        m_buffer->UpdateVertexArray(mesh, m_builder);
        m_buffer->UpdateVao(f);
    });
}

void GtTextScreenDrawable::draw(OpenGLFunctions* f)
{
    m_material.Draw(f);
    f->glPointSize(10.f);
}

void GtTextScreenDrawable::drawDepth(OpenGLFunctions* )
{
}

void GtTextScreenDrawable::onInitialize(OpenGLFunctions* f)
{
    m_buffer->Initialize(f);
}


DispatcherConnections GtTextDrawableSettings::ConnectFrom(const char* location, const GtTextDrawableSettings& another)
{
    DispatcherConnections result;
    result += Scale.ConnectFrom(location, another.Scale);
    result += Color.ConnectFrom(location, another.Color);
    result += BorderColor.ConnectFrom(location, another.BorderColor);
    result += BorderWidth.ConnectFrom(location, another.BorderWidth);
    result += Contrast.ConnectFrom(location, another.Contrast);
    result += UseDirectionCorrection.ConnectFrom(location, another.UseDirectionCorrection);
    result += Visible.ConnectFrom(location, another.Visible);
    return result;
}
