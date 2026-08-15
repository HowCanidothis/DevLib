#ifndef GTVIEWCONTEXT_H
#define GTVIEWCONTEXT_H

#include <QObject>
#include "gtrenderer.h"
#include "gtrenderercontroller.h"
#include "gtscene.h"

class GtViewContext
{
public:
    GtViewContext()
        : m_renderer(nullptr)
    {}

    // Constructor 1: Direct shared master scene assignment
    explicit GtViewContext(GtRenderer* r)
        : m_renderer(r)
        , m_scene(r->m_scene) // Assumes r is valid. Direct pointer extraction.
    {
    }

    GtViewContext(GtRenderer* r, const SP<GtScene>& scene)
        : m_renderer(r)
        , m_scene(scene)
    {
    }

    GtViewContext(GtRenderer* r, GtRendererController* c)
        : m_renderer(r)
        , m_scene(c->GetScene()) // Pure, zero-overhead assignment
    {
    }

    void UpdateFrame() const { m_renderer->UpdateFrame(); }

    GtRenderer* GetRenderer() const { return m_renderer; }
    const GtMeshLoader::Material* GetShadingMaterial(const Name& id) const { return m_renderer->GetShadingMaterial(id); }
    const GtFontPtr& GetFont(const Name& id) const { return m_renderer->GetFont(id); }
    GtShaderProgramPtr GetShaderProgram(const Name& id) const { return m_renderer->GetShaderProgram(id); }
    GtMaterialMeshResource GetMaterialMesh(const Name& id) const { return m_renderer->GetMaterialMesh(id); }

    template<class T, typename ... Args>
    T* CreateDrawableQueued(qint32 queueNumber, Args... args) const
    {
        auto* result = new T(*this, args...);
        const auto& targetScene = m_scene;
        auto* targetRenderer = m_renderer;

        m_renderer->Asynch([targetRenderer, targetScene, result, queueNumber] {
            result->initialize(targetRenderer);
            targetScene->AddDrawable(result, queueNumber);
        });
        return result;
    }

    template<class T, typename ... Args>
    T* CreateDrawable(Args... args) const
    {
        return CreateDrawableQueued<T, Args...>(m_renderer->m_queueNumber, args...);
    }

private:
    GtRenderer* m_renderer;
    SP<GtScene> m_scene;
};

#endif // GTVIEWCONTEXT_H
