#include "gtmaterialparametervector3f.h"

#include <QOpenGLShaderProgram>
#include "ResourcesModule/resourcessystem.h"
#include "GraphicsToolsModule/gtrenderer.h"

GtMaterialParameterVector2F::GtMaterialParameterVector2F(const QString& name, const Name& resource)
    : Super(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterVector2F::apply()
{
    m_vector = currentRenderer()->GetResource<Vector2F>(m_resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, m_vector->Data().Get());
    };
}


GtMaterialParameterVector3F::GtMaterialParameterVector3F(const QString& name, const Name& resource)
    : Super(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterVector3F::apply()
{
    m_vector = currentRenderer()->GetResource<Vector3F>(m_resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, m_vector->Data().Get());
    };
}
