#include "gtmaterialparametermatrix.h"

#include <QOpenGLShaderProgram>
#include "ResourcesModule/resourcessystem.h"
#include "GraphicsToolsModule/gtrenderer.h"

GtMaterialParameterMatrix::GtMaterialParameterMatrix(const QString& name, const Name& resource)
    : Super(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterMatrix::apply()
{
    m_matrix = currentRenderer()->GetResource<Matrix4>(m_resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, m_matrix->Data().Get());
    };
}
