#ifndef GTVIEWDELEGATEBASE_H
#define GTVIEWDELEGATEBASE_H

#include "gtobjectbase.h"
#include "ResourcesModule/resourcessystem.h"

template<class T> class Resource;
class GtTexture;
class GtShadowMapTechnique;
class GtFrameTexture;

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
    typedef std::function<void(QOpenGLShaderProgram* program, gLocID location, OpenGLFunctions* f)> FDelegate;
    GtMaterialParameterBase(const QString& name, const QString& resource);
    GtMaterialParameterBase(const QString& name, const FDelegate& delegate);
    virtual ~GtMaterialParameterBase();
protected:
    friend class GtMaterial;
    void bind(QOpenGLShaderProgram*, OpenGLFunctions* f);

    void installDelegate();
    virtual FDelegate apply();
    virtual void updateTextureUnit(gTexUnit&) {}
    void updateLocation(QOpenGLShaderProgram* program);

    template<class T> T* asObject() const { return (T*)ptr; }
protected:
    FDelegate delegate;
    gLocID location;
    QString name;
    Name resource;
    static GtMaterial*& material() { static GtMaterial* res; return res; }
};

#endif // GTVIEWDELEGATE_H
