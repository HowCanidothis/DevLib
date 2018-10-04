#include "gtmaterialparametershadow.h"

#include <QOpenGLShaderProgram>
#include "../gtshadowmaptechnique.h"
#include "../internal.hpp"
#include "ResourcesModule/resourcessystem.h"

GtMaterialParameterShadow::GtMaterialParameterShadow(const QString& name, const QString& resource)
    : Super(name, resource)
{

}

GtMaterialParameterBase::FDelegate GtMaterialParameterShadow::apply()
{
    technique = ResourcesSystem::GetResource<GtShadowMapTechnique>(resource);
    const auto& tech = technique->Data().Get();
    gTexID depth = tech.getDepthTexture();
    return [this, depth](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
        GtTexture2D::bindTexture(f, unit, depth);
        program->setUniformValue(loc, unit);
    };
}
