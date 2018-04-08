#include "gtmaterialmatrix.h"

#include <QOpenGLShaderProgram>
#include "ResourcesModule/resourcessystem.h"

#include "../internal.hpp"

GtMaterialMatrix::GtMaterialMatrix(const QString& name, const QString& resource)
    : GtMaterialBase(name, resource)
{}

GtMaterialBase::F_Delegate GtMaterialMatrix::apply()
{
    matrix = ResourcesSystem::getResource<Matrix4>(resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, *matrix->data());
    };
}
