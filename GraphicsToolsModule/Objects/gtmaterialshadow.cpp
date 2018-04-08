#include "gtmaterialshadow.h"

#include <QOpenGLShaderProgram>
#include "../gtshadowmaptechnique.h"
#include "../internal.hpp"
#include "ResourcesModule/resourcessystem.h"

GtMaterialShadow::GtMaterialShadow(const QString& name, const QString& resource)
    : Super(name, resource)
{

}

GtMaterialBase::F_Delegate GtMaterialShadow::apply()
{
    technique = ResourcesSystem::getResource<GtShadowMapTechnique>(resource);
    auto tech = technique->data();
    gTexID depth = tech->getDepthTexture();
    return [this, depth](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
        GtTexture2D::bindTexture(f, unit, depth);
        program->setUniformValue(loc, unit);
    };
}
