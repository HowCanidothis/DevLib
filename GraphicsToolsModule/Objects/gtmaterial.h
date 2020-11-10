#ifndef GTVIEWBASE_H
#define GTVIEWBASE_H

#include "gtobjectbase.h"
#include "SharedModule/array.h"
#include "SharedModule/stack.h"

class GtMesh;
class GtCamera;
class GtMaterialParameterBase;

class GtMaterial : public GtObjectBase
{
public:
    GtMaterial(gRenderType renderType, const GtShaderProgramPtr& program);
    virtual ~GtMaterial();

    void AddParameter(const SharedPointer<GtMaterialParameterBase>&);
    void AddMesh(const SharedPointer<GtMesh>& mesh);
    void Draw(OpenGLFunctions* f);
    void SetVisible(bool visible);

    void SetRenderType(gRenderType renderType);

    void Update();

private:
    void updateParameters(OpenGLFunctions* f);
    QByteArray extractShader(const QString& fileName) const;

protected:
    friend class GtMaterialParameterBase;

    QVector<SharedPointer<GtMaterialParameterBase>> m_parameters;
    QVector<SharedPointer<GtMesh>> m_meshs;

    QString m_shadersPath;
    gRenderType m_renderType;
    bool m_visible;
    std::atomic_bool m_isDirty;
    GtShaderProgramPtr m_shaderProgram;

    // GtObjectBase interface
public:
    void MapProperties(QtObserver*) override {}
};

#endif // GTVIEW_H
