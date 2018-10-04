#include "gtmaterialparametermatrix.h"

#include <QOpenGLShaderProgram>
#include "ResourcesModule/resourcessystem.h"

#include "../internal.hpp"

GtMaterialParameterMatrix::GtMaterialParameterMatrix(const QString& name, const QString& resource)
    : GtMaterialParameterBase(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterMatrix::apply()
{
    matrix = ResourcesSystem::GetResource<Matrix4>(resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, matrix->Data().Get());
    };
}
