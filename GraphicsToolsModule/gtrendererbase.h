#ifndef GTRENDERERBASE_H
#define GTRENDERERBASE_H

#include <QSurfaceFormat>

#include <SharedModule/internal.hpp>

class GtRendererBase : public ThreadComputingBase
{
    using Super = ThreadComputingBase;
public:
    GtRendererBase(GtRendererBase* sharedRenderer);
    ~GtRendererBase();

private:
    void run() override;
    void compute() override;

protected:
    virtual bool onInitialize() = 0;
    virtual void onDraw() = 0;
    virtual void onDestroy() = 0;

protected:
    static QMutex& initializationMutex();
    QSurfaceFormat m_surfaceFormat;
    ScopedPointer<QOpenGLContext> m_context;
    ScopedPointer<class QOffscreenSurface> m_surface;
    std::atomic_bool m_isInitialized;
    GtRendererBase* m_shareRenderer;
    std::atomic_bool m_isValid;
};

#endif // GTRENDERERBASE_H
