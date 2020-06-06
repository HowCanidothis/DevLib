#include "gtmaterialparametervector3f.h"

#include <QOpenGLShaderProgram>
#include "ResourcesModule/resourcessystem.h"

#include "../internal.hpp"

GtMaterialParameterVector3F::GtMaterialParameterVector3F(const QString& name, const QString& resource)
    : GtMaterialParameterBase(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterVector3F::apply()
{
    m_vector = ResourcesSystem::GetResource<Vector3F>(m_resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, m_vector->Data().Get());
    };
}
