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

    GtMaterial(gRenderType renderType);
    virtual ~GtMaterial();

    void AddParameter(GtMaterialParameterBase*);
    void AddMesh(GtMeshBase* mesh);
    void Draw(OpenGLFunctions* f);

    void SetDir(const QString& path) { m_shadersPath = path; }
    GtMaterial& AddShader(ShaderType type, const QString& file);
    void SetShaders(const QString& path, const QString& vertFile, const QString& fragFile);
    void SetShaders(const QString& path, const QString& vertFile, const QString& geomFile, const QString& fragFile);

    void Update();

protected:
    friend class GtMaterialParameterBase;
    struct Shader {
        QString File;
        qint32 Type;
    };

    Stack<GtMaterialParameterBase*> m_parameters;
    Array<GtMeshBase*> m_meshs;
    ScopedPointer<class QOpenGLShaderProgram> m_shaderProgram;
    ArrayPointers<Shader> m_shaders;
    QString m_shadersPath;
    gRenderType m_renderType;

    // GtObjectBase interface
public:
    void MapProperties(Observer* observer) Q_DECL_OVERRIDE;
};

#endif // GTVIEW_H
