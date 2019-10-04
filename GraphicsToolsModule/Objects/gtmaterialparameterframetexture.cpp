#include "gtmaterialparameterframetexture.h"

#ifdef OPENCV

#include <QOpenGLShaderProgram>

#include "../gtframetexture.h"

GtMaterialParameterFrameTexture::GtMaterialParameterFrameTexture(const QString& name, const QString& resource)
    : Super(name, resource)
{

}

GtMaterialParameterFrameTexture::FDelegate GtMaterialParameterFrameTexture::apply()
{
    m_frameTexture = ResourcesSystem::GetResource<GtFrameTexture>(m_resource);
    gTexID texture = m_frameTexture->Data().Get().getOutput()->GetId();
    return [this, texture](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
        GtTexture2D::bindTexture(f, m_unit, texture);
        program->setUniformValue(loc, m_unit);
    };
}

#endif
