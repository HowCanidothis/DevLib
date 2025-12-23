#include "gtmaterial.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include "gtmaterialparameterbase.h"
#include "../gtmeshbase.h"
#include "../gtrenderer.h"
#include "gtshaderprogram.h"

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
        parameter->updateLocation(m_shaderProgram->GetQOpenGLShaderProgram());
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
