#include "gtmaterialparametershadow.h"

#include <QOpenGLShaderProgram>
#include "../gtshadowmaptechnique.h"
#include "ResourcesModule/resourcessystem.h"
#include "GraphicsToolsModule/gtrenderer.h"
#include "GraphicsToolsModule/gttexture2D.h"
#include "GraphicsToolsModule/gtframebufferobject.h"
#include "GraphicsToolsModule/gtcamera.h"

GtMaterialParameterShadow::GtMaterialParameterShadow(const QString& name, const Name& resource)
    : Super(name, resource)
{

}

GtMaterialParameterBase::FDelegate GtMaterialParameterShadow::apply()
{
    m_technique = currentRenderer()->GetResource<GtShadowMapTechnique>(m_resource);
    gTexID depth;
    m_technique.GetAccess([&](GtShadowMapTechnique& tech){
        depth = tech.GetDepthTexture();
    });
    return [this, depth](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
        GtTexture2D::bindTexture(f, m_unit, depth);
        program->setUniformValue(loc, m_unit);
    };
}
