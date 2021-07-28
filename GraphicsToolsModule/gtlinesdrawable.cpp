#include "gtlinesdrawable.h"

#include "Objects/gtmaterial.h"
#include "Objects/gtmaterialparametermatrix.h"
#include "gtmeshbase.h"

GtLinesDrawable::GtLinesDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram)
    : Super(renderer)
    , Width(1.f)
    , m_buffer(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Point3F, QOpenGLBuffer::StaticDraw))
    , m_material(::make_scoped<GtMaterial>(GL_LINES, shaderProgram))
{
    m_material->AddParameter(::make_shared<GtMaterialParameterMatrix>("MVP", "mvp"));
    m_material->AddParameter(::make_shared<GtMaterialParameterBase>("COLOR", &Color.Native()));
    m_material->AddParameter(::make_shared<GtMaterialParameterBase>("MODEL_MATRIX", &Transform.Native()));
    m_material->AddMesh(::make_shared<GtMesh>(m_buffer));

    Points.SetSetterHandler(CreateThreadHandler());
    Points.OnChange.Connect(this, [this]{
        m_buffer->UpdateVertexArray(Points);
    });
}

void GtLinesDrawable::drawDepth(OpenGLFunctions*)
{
}

void GtLinesDrawable::draw(OpenGLFunctions* f)
{
    if(!Visible) {
        return;
    }

    f->glLineWidth(Width);
    m_material->Draw(f);
    f->glLineWidth(1.f);
}

void GtLinesDrawable::onInitialize(OpenGLFunctions* f)
{
    m_buffer->Initialize(f);
}

void GtLinesDrawable::onDestroy(OpenGLFunctions*)
{
}
