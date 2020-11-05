#include "gttextdrawable.h"

#include "Objects/gtmaterialparametermatrix.h"
#include "Objects/gtmaterialparametervector3f.h"
#include "Objects/gtmaterialparametertexture.h"
#include "gttexture2D.h"

void GtTextMap::LoadFromFnt(const QString& fntFile)
{
    m_glyphs.clear();
    QFile file(fntFile);
    if(file.open(QFile::ReadOnly)) {
        auto contents = file.readAll();
        QRegExp regexp(R"((\w+)=([^\s]+))");
        qint32 pos = 0;

        qint32 id;
        GtTextGlyph currentGlyph;
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
        { Name("height"), [&currentGlyph](const QString& ref) { currentGlyph.Height = ref.toFloat(); } },
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

GtTextDrawable::GtTextDrawable(GtRenderer* renderer, const GtTextMap& map)
    : Super(renderer)
    , Scale(0.13)
    , Color(QColor(Qt::white))
    , BorderWidth(0.2)
    , Contrast(3.0)
    , UseDirectionCorrection(false)
    , Visible(true)
    , m_material(GL_POINTS)
    , m_buffer(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Custom, QOpenGLBuffer::StaticDraw))
    , m_textMap(map)
{
    m_material.AddMesh(::make_shared<GtMesh>(m_buffer));
    m_material.AddParameter(::make_shared<GtMaterialParameterMatrix>("MVP", "mvp"));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("SCREEN_SIZE", [](QOpenGLShaderProgram* p, gLocID location, OpenGLFunctions*){
                                auto screenSize = ResourcesSystem::GetResource<Vector2F>("screenSize");
                                p->setUniformValue(location, screenSize->Data().Get());
                            }));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("ENABLE_DIR_CORRECTION", &UseDirectionCorrection.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("BORDER_WIDTH", &BorderWidth.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("CONTRAST", &Contrast.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("TEXT_SCALE", &Scale.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("COLOR", &Color.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterBase>("BORDER_COLOR", &BorderColor.Native()));
    m_material.AddParameter(::make_shared<GtMaterialParameterTexture>("TEXTURE", "FontTexture"));
    m_material.SetShaders(GT_SHADERS_PATH, "sdftext.vert", "sdftext.geom", "sdftext.frag");

    m_builder.AddComponent<float>(3);
    m_builder.AddComponent<float>(4);
    m_builder.AddComponent<float>(2);
    m_builder.AddComponent<float>(4);
    m_builder.AddComponent<float>(3);
    m_builder.AddComponent<float>(4);

    Visible.Subscribe([this]{
        Update([this]{
            m_material.SetVisible(Visible);
        });
    });
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
                auto glyphInfo = m_textMap.GetGlyph(glyph.unicode());
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

void GtTextDrawable::GenerateProperties(const QString& prefix)
{
    m_properties = {
        PropertyFromLocalProperty::Create(Name(prefix + "/Scale"), Scale),
        PropertyFromLocalProperty::Create(Name(prefix + "/Color"), Color),
        PropertyFromLocalProperty::Create(Name(prefix + "/BorderColor"), BorderColor),
        PropertyFromLocalProperty::Create(Name(prefix + "/BorderWidth"), BorderWidth),
        PropertyFromLocalProperty::Create(Name(prefix + "/Contrast"), Contrast),
        PropertyFromLocalProperty::Create(Name(prefix + "/UseDirectionCorrection"), UseDirectionCorrection)
    };
}

void GtTextDrawable::ConnectFrom(GtTextDrawable* another)
{
    Scale.ConnectFrom(another->Scale);
    Color.ConnectFrom(another->Color);
    BorderColor.ConnectFrom(another->BorderColor);
    BorderWidth.ConnectFrom(another->BorderWidth);
    Contrast.ConnectFrom(another->Contrast);
    UseDirectionCorrection.ConnectFrom(another->UseDirectionCorrection);
}

void GtTextDrawable::draw(OpenGLFunctions* f)
{
    f->glDisable(GL_DEPTH_TEST);
    m_material.Draw(f);
    f->glEnable(GL_DEPTH_TEST);
    f->glPointSize(10.f);
}

void GtTextDrawable::drawDepth(OpenGLFunctions* )
{
}

void GtTextDrawable::onInitialize(OpenGLFunctions* f)
{
    m_material.Update();
    m_buffer->Initialize(f);
}
