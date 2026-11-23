#ifndef GTOBJECTBASE_H
#define GTOBJECTBASE_H

#include <atomic>

#include "SharedGuiModule/decl.h"
#include "PropertiesModule/internal.hpp"
#include <SharedModule/External/external.hpp>
#include <SharedModule/internal.hpp>

#include "GraphicsToolsModule/decl.h"

class GtObjectBase
{
public:
    virtual ~GtObjectBase(){}
    virtual void MapProperties(QtObserver* ) {}
};

class GtDrawableBase : public GtObjectBase ATTACH_MEMORY_SPY_2(GtDrawableBase)
{
public:
    GtDrawableBase(const class GtViewContext& viewContext);
    ~GtDrawableBase();

    DispatcherConnectionsSafe Connections;

    void AboutToDestroy();
    AsyncResult Destroy();
    AsyncResult Update(const std::function<void (OpenGLFunctions*)>& f);
    AsyncResult Update(const FAction& f);

    template<typename T1, typename T2, typename ... Args>
    void MoveToThread(T1& f, T2& s, Args&... args)
    {
        adapters::Combine([this](auto& p) {
            MoveToThread(p);
        }, f, s, args...);
    }
    template<typename ... Args>
    void MoveToThread(CommonDispatcher<Args...>& d)
    {
        d.SetAutoThreadSafe();
    }

    template<typename T>
    void MoveToThread(LocalProperty<T>& p)
    {
        p.OnChanged.SetAutoThreadSafe();
        p.SetSetterHandler(m_threadHandler);
    }

    ThreadHandler GetThreadHandler() const { return m_threadHandler; }
    ThreadHandlerNoThreadCheck GetThreadHandlerNoCheck() const { return m_threadHandlerNoCheck; }

    template<class T>
    T* As() { return reinterpret_cast<T*>(this); }

    // Show flags of the drawable (WPS: WPSVisualizationShowFlag): drawn only in views whose show flags contain all of them
    // (GtRendererController::SetShowFlags). 0 - drawn in every view. Any thread.
    void SetShowFlag(qint32 flag) { m_showFlag = flag; }
    // Visibility key (WPS: id of the visualization object; every drawable of the object gets the same key): a view hides
    // the drawable while the key is in its hidden keys (GtRendererController::SetHiddenKeys). Empty - never hidden by key.
    // Stored as the 64-bit value of the name (Name::operator qint64): any thread, no lock.
    void SetVisibilityKey(const Name& key) { m_visibilityKey = key.IsNull() ? 0 : qint64(key); }
    qint64 GetVisibilityKey() const { return m_visibilityKey; }
    // Owner view (GtRendererController::SetViewId): drawn only in that view. 0 - every view. Any thread.
    void SetOwnerView(qint64 viewId) { m_ownerView = viewId; }
    qint64 GetOwnerView() const { return m_ownerView; }

protected:
    friend class GtViewContext;
    friend class GtScene;
    friend class GtRenderer;
    friend class GtRendererController;
    friend class GtRenderPath;
    virtual void drawDepth(OpenGLFunctions* f) { draw(f); }
    virtual void preDraw(OpenGLFunctions*) {};
    virtual void draw(OpenGLFunctions* f) = 0;
    void initialize(class GtRenderer* renderer);
    virtual void onInitialize(OpenGLFunctions* f) = 0;
    virtual void onDestroy(OpenGLFunctions* f) = 0;
    virtual void onAboutToDestroy() = 0;
    virtual bool isCustomRenderStageAccepted(const Name& renderStageId) const { return false; }

    void delayedDraw(const FAction& draw);
    void enableDepthTest();
    void disableDepthTest();
    bool isVisibleFromMask(qint32 mask) const;
    // Flag of the current view's show flags (GtRendererController::SetShowFlags).
    bool isShown(qint32 flag) const;
    // Shown in the current view: SetShowFlag (every bit required), SetOwnerView and SetVisibilityKey against the view
    // (GtRendererController::IsShown); the scene skips the drawable otherwise.
    bool isShownInCurrentView() const;
    template<typename ... Args>
    auto isVisibleFromMasks(const Args&... masks) const
    {
        return std::make_tuple((isVisibleFromMask(masks))...);
    }

    const GtRenderProperties& getRenderProperties() const;
    const GtRendererController* getRendererController() const;

protected:
    GtRenderer* m_renderer;
    GtScene* m_scene;
    SharedPointer<std::atomic_bool> m_destroyed;
    bool m_rendererDrawable;
    ThreadHandler m_threadHandler;
    std::atomic<qint32> m_showFlag{0}; // SetShowFlag
    std::atomic<qint64> m_ownerView{0}; // SetOwnerView
    std::atomic<qint64> m_visibilityKey{0}; // SetVisibilityKey; 0 - none
    ThreadHandlerNoThreadCheck m_threadHandlerNoCheck;
};

inline void GtDrawableDeleter::operator()(GtDrawableBase* obj)
{
    obj->AboutToDestroy();
    obj->Destroy();
}

#endif // GTOBJECTBASE_H
