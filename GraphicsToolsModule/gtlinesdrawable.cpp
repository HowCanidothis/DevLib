#include "gtlinesdrawable.h"

#include "Objects/gtmaterial.h"
#include "Objects/gtmaterialparametermatrix.h"
#include "gtmeshbase.h"
#include "gtrenderer.h"

GtLinesDrawable::GtLinesDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram)
    : Super(renderer)
    , Visible(0xffffffff)
    , Width(1.f)
    , m_buffer(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Point3F, QOpenGLBuffer::StaticDraw))
    , m_material(::make_scoped<GtMaterial>(GL_LINES, shaderProgram))
{
    m_material->AddParameter(::make_shared<GtMaterialParameterMatrix>("MVP", GtNames::mvp));
    m_material->AddParameter(::make_shared<GtMaterialParameterBase>("COLOR", &Color.Native()));
    m_material->AddParameter(::make_shared<GtMaterialParameterBase>("MODEL_MATRIX", &Transform.Native()));
    m_material->AddMesh(::make_shared<GtMesh>(m_buffer));

    MoveToThread(Points);
}

void GtLinesDrawable::drawDepth(OpenGLFunctions*)
{
}

void GtLinesDrawable::draw(OpenGLFunctions* f)
{
    if(!isVisibleFromMask(Visible)) {
        return;
    }

    f->glLineWidth(Width);
    m_material->Draw(f);
    f->glLineWidth(1.f);
}

void GtLinesDrawable::onInitialize(OpenGLFunctions* f)
{
    Points.OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this]{
        m_buffer->UpdateVertexArray(Points);
    });

    m_buffer->Initialize(f);
}

void GtLinesDrawable::onDestroy(OpenGLFunctions*)
{
}
