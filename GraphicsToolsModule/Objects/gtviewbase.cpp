#include "gtviewbase.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include "gtmaterialbase.h"
#include "../gtmeshbase.h"

GtViewBase::GtViewBase()
{

}

GtViewBase::~GtViewBase()
{

}

void GtViewBase::addMaterial(GtMaterialBase* delegate)
{
    materials.push(delegate);
}

void GtViewBase::addMesh(GtMeshBase* mesh)
{
    meshs.append(mesh);
}

void GtViewBase::draw(OpenGLFunctions* f)
{
    Q_ASSERT(shader_program != nullptr);
    shader_program->bind();

    for(GtMaterialBase* material : materials)
        material->bind(shader_program.data(), f);

    for(GtMeshBase* mesh : meshs) {
        if(mesh->isVisible())
            mesh->draw(f);
    }

    shader_program->release();
}

GtViewBase&GtViewBase::addShader(GtViewBase::ShaderType type, const QString& file)
{
    shaders.append(new Shader({file, type}));
    return *this;
}

void GtViewBase::setShaders(const QString& path, const QString& vert_file, const QString& frag_file)
{
    setDir(path);
    addShader(Vertex, vert_file).
            addShader(Fragment, frag_file);
    update();
}

void GtViewBase::update()
{
    LOGOUT;
    shader_program.reset(new QOpenGLShaderProgram);
    shader_program->create();
    {
        DirBinder dir(shaders_path);
        for(Shader* shader : shaders) {
            QOpenGLShader* shader_object = new QOpenGLShader((QOpenGLShader::ShaderTypeBit)shader->type, shader_program.data());
            if(shader_object->compileSourceFile(shader->file)) {
                shader_program->addShader(shader_object);
            }
        }
    }
    if(!shader_program->link()) {
        log.error() << "unable to link program" << shader_program->log();
    }

    gTexUnit unit = 0;

    for(GtMaterialBase* material : materials) {
        material->updateLocation(shader_program.data());
        material->updateTextureUnit(unit);
        material->installDelegate();
    }
}

void GtViewBase::mapProperties(Observer* observer)
{
    qint32 counter = 0;
    for(Shader* shader : shaders) {
        new TextFileNamePropertyPtr("Shaders/" + QString::number(counter++), &shader->file);
        observer->addFileObserver(&shaders_path, &shader->file, [this]{
            this->update();
        });
    }

    GtMaterialBase::view() = this;

    for(GtMaterialBase* material : materials) {
        material->mapProperties(observer);
    }
}
