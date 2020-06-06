#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include "SharedGuiModule/decl.h"
#include "PropertiesModule/internal.hpp"
#include <SharedModule/External/external.hpp>
#include <SharedModule/internal.hpp>

class GtObjectBase
{
public:
    virtual void MapProperties(Observer* ) {}
};

class GtDrawableBase : public GtObjectBase
{
public:
    GtDrawableBase()
        : m_initialized(false)
    {}
    virtual ~GtDrawableBase() {}

    virtual void Draw(OpenGLFunctions* f) = 0;
    void Initialize(OpenGLFunctions* f)
    {
        if(!m_initialized) {
            onInitialize(f);
            m_initialized = true;
        }
    }
    bool IsInitialized() const { return m_initialized; }

protected:
    virtual void onInitialize(OpenGLFunctions* f) = 0;

private:
    bool m_initialized;
};

class GtInteractableBase : public GtDrawableBase
{
public:
    virtual ~GtInteractableBase() {}

    virtual bool ContainsPoint(const Point3F& point) const = 0;
    virtual float DistanceToPoint(const Point3F& point) const = 0;
};


#endif // GTOBJECTBASE_H
