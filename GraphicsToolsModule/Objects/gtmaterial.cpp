#include "gtmaterial.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include "gtmaterialparameterbase.h"
#include "../gtmeshbase.h"

GtMaterial::GtMaterial()
{

}

GtMaterial::~GtMaterial()
{

}

void GtMaterial::addParameter(GtMaterialParameterBase* delegate)
{
    parameters.Push(delegate);
}

void GtMaterial::addMesh(GtMeshBase* mesh)
{
    meshs.Append(mesh);
}

void GtMaterial::draw(OpenGLFunctions* f)
{
    Q_ASSERT(shader_program != nullptr);
    shader_program->bind();

    for(GtMaterialParameterBase* parameter : parameters)
        parameter->bind(shader_program.data(), f);

    for(GtMeshBase* mesh : meshs) {
        if(mesh->isVisible())
            mesh->draw(f);
    }

    shader_program->release();
}

GtMaterial&GtMaterial::addShader(GtMaterial::ShaderType type, const QString& file)
{
    shaders.Append(new Shader({file, type}));
    return *this;
}

void GtMaterial::setShaders(const QString& path, const QString& vert_file, const QString& frag_file)
{
    setDir(path);
    addShader(Vertex, vert_file).
            addShader(Fragment, frag_file);
    update();
}

void GtMaterial::update()
{
    shader_program.reset(new QOpenGLShaderProgram);
    shader_program->create();
    {
        DirBinder dir(shaders_path);
        for(Shader* shader : shaders) {
            QOpenGLShader* shader_object = new QOpenGLShader((QOpenGLShader::ShaderTypeBit)shader->Type, shader_program.data());
            if(shader_object->compileSourceFile(shader->File)) {
                shader_program->addShader(shader_object);
            }
        }
    }
    if(!shader_program->link()) {
        qCCritical(LC_SYSTEM) << "unable to link program" << shader_program->log();
    }

    gTexUnit unit = 0;

    for(GtMaterialParameterBase* parameter : parameters) {
        parameter->updateLocation(shader_program.data());
        parameter->updateTextureUnit(unit);
        parameter->installDelegate();
    }
}

void GtMaterial::mapProperties(Observer* observer)
{
    qint32 counter = 0;
    for(Shader* shader : shaders) {
        new ExternalTextFileNameProperty(Name("Shaders/" + QString::number(counter++)), shader->File);
        observer->AddFilePtrObserver(&shaders_path, &shader->File, [this]{
            this->update();
        });
    }

    GtMaterialParameterBase::material() = this;

    for(GtMaterialParameterBase* parameter : parameters) {
        parameter->mapProperties(observer);
    }
}
