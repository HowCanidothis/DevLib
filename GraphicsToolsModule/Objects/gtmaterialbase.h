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
class GtViewBase;

class GtMaterialBase : public GtObjectBase
{
public:
    typedef std::function<void(QOpenGLShaderProgram* program, gLocID location, OpenGLFunctions* f)> F_Delegate;
    GtMaterialBase(const QString& name, const QString& resource);
    GtMaterialBase(const QString& name, const F_Delegate& delegate);
    virtual ~GtMaterialBase();
protected:
    friend class GtViewBase;
    void bind(QOpenGLShaderProgram*, OpenGLFunctions* f);

    void installDelegate();
    virtual F_Delegate apply();
    virtual void updateTextureUnit(gTexUnit&) {}
    void updateLocation(QOpenGLShaderProgram* program);

    template<class T> T* asObject() const { return (T*)ptr; }
protected:
    F_Delegate delegate;
    gLocID location;
    QString name;
    QString resource;
    static GtViewBase*& view() { static GtViewBase* res; return res; }
};

#endif // GTVIEWDELEGATE_H
