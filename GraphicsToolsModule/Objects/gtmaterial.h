#ifndef GTVIEWBASE_H
#define GTVIEWBASE_H

#include "gtobjectbase.h"
#include "SharedModule/array.h"
#include "SharedModule/stack.h"

class GtMeshBase;
class GtCamera;
class GtMaterialParameterBase;

class GtMaterial : public GtObjectBase
{
public:
    enum ShaderType {
        Vertex                 = 0x0001,
        Fragment               = 0x0002,
        Geometry               = 0x0004,
        TessellationControl    = 0x0008,
        TessellationEvaluation = 0x0010,
        Compute                = 0x0020
    };

    GtMaterial();
    virtual ~GtMaterial();

    void addParameter(GtMaterialParameterBase*);
    void addMesh(GtMeshBase* mesh);
    void draw(OpenGLFunctions* f);

    void setDir(const QString& path) { shaders_path = path; }
    GtMaterial& addShader(ShaderType type, const QString& file);
    void setShaders(const QString& path, const QString& vert_file, const QString& frag_file);

    void update();
protected:
    friend class GtMaterialParameterBase;
    struct Shader {
        QString File;
        qint32 Type;
    };

    Stack<GtMaterialParameterBase*> parameters;
    Array<GtMeshBase*> meshs;
    ScopedPointer<class QOpenGLShaderProgram> shader_program;
    ArrayPointers<Shader> shaders;
    QString shaders_path;
    // GtObjectBase interface
public:
    void mapProperties(Observer* observer) Q_DECL_OVERRIDE;
};

#endif // GTVIEW_H
