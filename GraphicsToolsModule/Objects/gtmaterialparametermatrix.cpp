#include "gtmaterialparametermatrix.h"

#include <QOpenGLShaderProgram>
#include "ResourcesModule/resourcessystem.h"

#include "../internal.hpp"

GtMaterialParameterMatrix::GtMaterialParameterMatrix(const QString& name, const QString& resource)
    : GtMaterialParameterBase(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterMatrix::apply()
{
    m_matrix = ResourcesSystem::GetResource<Matrix4>(m_resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, m_matrix->Data().Get());
    };
}
