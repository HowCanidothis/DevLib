#include "gtmaterial.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include "gtmaterialparameterbase.h"
#include "../gtmeshbase.h"
#include "../gtrenderer.h"
#include "gtshaderprogram.h"
#include "gtmaterialparametertexture.h"

GtMaterial::GtMaterial(gRenderType renderType, const GtShaderProgramPtr& program)
    : m_renderType(renderType)
    , m_visible(true)
    , m_isDirty(true)
    , m_shaderProgram(program)
{
    Q_ASSERT(m_shaderProgram != nullptr);
    program->OnUpdated += { this, [this]{
        m_isDirty = true;
    }};
}

GtMaterial::~GtMaterial()
{
    m_shaderProgram->OnUpdated -= this;
}

void GtMaterial::ApplyShadingMaterial(const GtMeshLoader::Material& shading)
{
    // 2. Map Scalar Properties via uniform parameters
    AddParameter(make_shared<GtMaterialParameterConst>("Ns", shading.Ns), false);
    AddParameter(make_shared<GtMaterialParameterConst>("Ni", shading.Ni), false);
    AddParameter(make_shared<GtMaterialParameterConst>("d", shading.D), false);

    // 3. Map Color Constants via vector uniform parameters
    AddParameter(make_shared<GtMaterialParameterConst>("Ka", shading.Ka));
    AddParameter(make_shared<GtMaterialParameterConst>("Kd", shading.Kd));
    AddParameter(make_shared<GtMaterialParameterConst>("Ks", shading.Ks));
    AddParameter(make_shared<GtMaterialParameterConst>("Ke", shading.Ke), false);

    // 4. Map Modern PBR Constant Extensions via uniform scalar parameters
    AddParameter(make_shared<GtMaterialParameterConst>("Pr", shading.Pr), false);
    AddParameter(make_shared<GtMaterialParameterConst>("Pm", shading.Pm), false);
    AddParameter(make_shared<GtMaterialParameterConst>("Ps", shading.Ps), false);
    AddParameter(make_shared<GtMaterialParameterConst>("Pc", shading.Pc), false);
    AddParameter(make_shared<GtMaterialParameterConst>("Pcr", shading.Pcr), false);

    // 4. Map Illumination Model integer property
    AddParameter(make_shared<GtMaterialParameterConst>("illum", static_cast<int>(shading.Ellum)), false);

    // 5. Dynamic Texture Flags - Include Emissive tracking
    bool hasKd   = !shading.MapKdFileName.IsNull();
    bool hasD    = !shading.MapDFileName.IsNull();
    bool hasBump = !shading.MapBumpFileName.IsNull();
    bool hasKs   = !shading.MapKsFileName.IsNull();
    bool hasKe   = !shading.MapKeFileName.IsNull();
    bool hasPm   = !shading.MapPmFileName.IsNull();
    bool hasPr   = !shading.MapPrFileName.IsNull();
    bool hasKn   = !shading.MapKnFileName.IsNull();

    // Verify if Metalness and Roughness target the exact same channel-packed asset map file
    bool isPackedPmr = (hasPm && hasPr && (shading.MapPmFileName == shading.MapPrFileName));

    AddParameter(make_shared<GtMaterialParameterConst>("hasMapKd", hasKd), false);
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapD", hasD), false);
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapBump", hasBump), false);
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapKs", hasKs), false);
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapPm", hasPm && !isPackedPmr), false);
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapPr", hasPr && !isPackedPmr), false);
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapPmr", isPackedPmr), false); // Combined flag
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapKn", hasKn), false);
    AddParameter(make_shared<GtMaterialParameterConst>("hasMapKe", hasKe), false);

    // 6. Map Texture File Resource References
    if (!shading.MapKaFileName.IsNull()) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_Ka", shading.MapKaFileName));
    }
    if (hasKd) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_Kd", shading.MapKdFileName));
    }
    if (hasKs) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_Ks", shading.MapKsFileName));
    }
    if (hasKe) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_Ke", shading.MapKeFileName));
    }
    if (hasD) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_d", shading.MapDFileName));
    }
    if (hasBump) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_Bump", shading.MapBumpFileName));
    }

    // Core Router: Bind single unified texture map if asset is channel-packed
    if (isPackedPmr) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_Pmr", shading.MapPmFileName));
    } else {
        if (hasPm) {
            AddParameter(make_shared<GtMaterialParameterTexture>("map_Pm", shading.MapPmFileName));
        }
        if (hasPr) {
            AddParameter(make_shared<GtMaterialParameterTexture>("map_Pr", shading.MapPrFileName));
        }
    }

    if (hasKn) {
        AddParameter(make_shared<GtMaterialParameterTexture>("map_Kn", shading.MapKnFileName));
    }
}


void GtMaterial::AddParameter(const SharedPointer<GtMaterialParameterBase>& parameter, bool required)
{
    parameter->SetRequired(required);
    m_parameters.append(parameter);
}

void GtMaterial::AddMesh(const GtMeshPtr& mesh)
{
    m_meshs.append(mesh);
}

void GtMaterial::Draw(OpenGLFunctions* f)
{
    if(!m_visible || !m_shaderProgram->IsValid()) {
        return;
    }

    if(!m_shaderProgram->Bind()) {
        return;
    }
    if(m_isDirty) {
        update();
    }

    updateParameters(f);

    for(const auto& mesh : m_meshs) {
        if(mesh->IsVisible())
            mesh->Draw(m_renderType, f);
    }

    m_shaderProgram->Release();
}

void GtMaterial::DrawInstanced(OpenGLFunctions* f, qint32 instanceCount)
{
    if(!m_visible || !m_shaderProgram->IsValid()) {
        return;
    }

    if(!m_shaderProgram->Bind()) {
        return;
    }
    if(m_isDirty) {
        update();
    }

    updateParameters(f);

    for(const auto& mesh : m_meshs) {
        if(mesh->IsVisible())
            mesh->DrawInstanced(m_renderType, f, instanceCount);
    }

    m_shaderProgram->Release();
}

void GtMaterial::SetVisible(bool visible)
{
    m_visible = visible;
}

void GtMaterial::SetRenderType(gRenderType renderType)
{
    m_renderType = renderType;
}

void GtMaterial::update()
{
    gTexUnit unit = 0;

    for(const auto& parameter : m_parameters) {
        parameter->updateLocation(m_shaderProgram->GetQOpenGLShaderProgram(), m_shaderProgram.get());
        parameter->updateTextureUnit(unit);
        parameter->installDelegate();
    }
    m_isDirty = false;
}

void GtMaterial::updateParameters(OpenGLFunctions* f)
{
    for(const auto& parameter : m_parameters) {
        parameter->bind(m_shaderProgram->GetQOpenGLShaderProgram(), f);
    }
}
