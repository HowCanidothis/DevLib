#ifndef GTVIEWDELEGATEBASE_H
#define GTVIEWDELEGATEBASE_H

#include <QOpenGLShaderProgram>

#include <ResourcesModule/internal.hpp>
#include <SharedModule/internal.hpp>

#include "gtobjectbase.h"

template<class T> class TResource;
class GtTexture;
class GtShadowMapTechnique;
class GtFrameTexture;

typedef TResource<Vector2F> Vector2FResource;
typedef TResource<Vector3F> Vector3FResource;
typedef TResource<Matrix4> Matrix4Resource;
typedef TResource<GtTexture> GtTextureResource;
typedef TResource<GtFrameTexture> GtFrameTextureResource;
typedef TResource<GtShadowMapTechnique> GtShadowMapTechniqueResource;

#ifdef WORK
#include <functional>
#endif

class GtMaterial;

class GtMaterialParameterBase : public GtObjectBase
{
protected:
    GtMaterialParameterBase(const QString& name)
        : m_name(name)
        , m_required(true)
    {}
public:
    typedef std::function<void(QOpenGLShaderProgram* program, gLocID m_location, OpenGLFunctions* f)> FDelegate;

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

    void SetRequired(bool required);

protected:
    friend class GtMaterial;
    void bind(QOpenGLShaderProgram* program, OpenGLFunctions* f);

    class GtRenderer* currentRenderer();
    void installDelegate();
    virtual FDelegate apply();
    virtual void updateTextureUnit(gTexUnit&) {}
    void updateLocation(const QOpenGLShaderProgram* program);

protected:
    FDelegate m_delegate;
    QString m_name;
    bool m_required;

    QHash<const QOpenGLShaderProgram*, gLocID> m_locations;
    static GtMaterial*& material() { static GtMaterial* res; return res; }
};

class GtMaterialResourceParameterBase : public GtMaterialParameterBase
{
    using Super = GtMaterialParameterBase;
public:
    GtMaterialResourceParameterBase(const QString& name, const Name& resource);

protected:
    Name m_resource;
};

class GtMaterialParameterConst : public GtMaterialParameterBase
{
    using Super = GtMaterialParameterBase;
public:
    template<class T>
    GtMaterialParameterConst(const QString& name, const T& value)
        : Super(name, [value](QOpenGLShaderProgram* p, gLocID location, OpenGLFunctions*){
            p->setUniformValue(location, value);
        })
    {}
};

using GtMaterialParameterBasePtr = SharedPointer<GtMaterialParameterBase>;

#endif // GTVIEWDELEGATE_H
