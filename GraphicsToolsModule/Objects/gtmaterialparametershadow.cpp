#include "gtmaterialparametershadow.h"

#include <QOpenGLShaderProgram>
#include "../gtshadowmaptechnique.h"
#include "../internal.hpp"
#include "ResourcesModule/resourcessystem.h"

GtMaterialParameterShadow::GtMaterialParameterShadow(const QString& name, const Name& resource)
    : Super(name, resource)
{

}

GtMaterialParameterBase::FDelegate GtMaterialParameterShadow::apply()
{
    m_technique = currentRenderer()->GetResource<GtShadowMapTechnique>(m_resource);
    const auto& tech = m_technique->Data().Get();
    gTexID depth = tech.GetDepthTexture();
    return [this, depth](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
        GtTexture2D::bindTexture(f, m_unit, depth);
        program->setUniformValue(loc, m_unit);
    };
}
