#ifndef GTVIEWDELEGATEBASE_H
#define GTVIEWDELEGATEBASE_H

#include <QOpenGLShaderProgram>

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

class GtMaterial;

class GtMaterialParameterBase : public GtObjectBase
{
public:
    typedef std::function<void(QOpenGLShaderProgram* program, gLocID m_location, OpenGLFunctions* f)> FDelegate;
    GtMaterialParameterBase(const QString& name, const Name& resource);
    GtMaterialParameterBase(const QString& name, const FDelegate& delegate);
    template<class T>
    GtMaterialParameterBase(const QString& name, const T* value)
        : m_delegate([value](QOpenGLShaderProgram* p, gLocID location, OpenGLFunctions*){
            p->setUniformValue(location, *value);
        })
        , m_name(name)
    {}
    GtMaterialParameterBase(const QString& name, const QColor* color)
        : m_delegate([color](QOpenGLShaderProgram* p, gLocID location, OpenGLFunctions*){
            p->setUniformValue(location, QVector4D(color->redF(), color->greenF(), color->blueF(), color->alphaF()));
        })
        , m_name(name)
    {}
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
