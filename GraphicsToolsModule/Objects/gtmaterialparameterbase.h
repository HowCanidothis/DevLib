#ifndef GTVIEWDELEGATEBASE_H
#define GTVIEWDELEGATEBASE_H

#include <SharedModule/internal.hpp>

#include "gtobjectbase.h"
#include "ResourcesModule/resourcessystem.h"

template<class T> class Resource;
class GtTexture;
class GtShadowMapTechnique;
class GtFrameTexture;

typedef Resource<Vector3F> Vector3FResource;
typedef Resource<Matrix4> Matrix4Resource;
typedef Resource<GtTexture> GtTextureResource;
typedef Resource<GtFrameTexture> GtFrameTextureResource;
typedef Resource<GtShadowMapTechnique> GtShadowMapTechniqueResource;

#ifdef WORK
#include <functional>
#endif

class QOpenGLShaderProgram;
class GtMaterial;

class GtMaterialParameterBase : public GtObjectBase
{
public:
    typedef std::function<void(QOpenGLShaderProgram* program, gLocID m_location, OpenGLFunctions* f)> FDelegate;
    GtMaterialParameterBase(const QString& name, const QString& resource);
    GtMaterialParameterBase(const QString& name, const FDelegate& delegate);
    virtual ~GtMaterialParameterBase();

protected:
    friend class GtMaterial;
    void bind(QOpenGLShaderProgram* program, OpenGLFunctions* f);

    void installDelegate();
    virtual FDelegate apply();
    virtual void updateTextureUnit(gTexUnit&) {}
    void updateLocation(QOpenGLShaderProgram* program);

protected:
    FDelegate m_delegate;
    QString m_name;
    Name m_resource;
    QHash<QOpenGLShaderProgram*, gLocID> m_locations;
    static GtMaterial*& material() { static GtMaterial* res; return res; }
};

using GtMaterialParameterBasePtr = SharedPointer<GtMaterialParameterBase>;

#endif // GTVIEWDELEGATE_H
