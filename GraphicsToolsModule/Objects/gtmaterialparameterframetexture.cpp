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
    _frameTexture = ResourcesSystem::GetResource<GtFrameTexture>(resource);
    gTexID texture = _frameTexture->Data()->getOutput()->getID();
    return [this, texture](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
        GtTexture2D::bindTexture(f, unit, texture);
        program->setUniformValue(loc, unit);
    };
}

#endif
