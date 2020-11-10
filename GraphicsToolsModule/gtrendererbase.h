#ifndef GTRENDERERBASE_H
#define GTRENDERERBASE_H

#include <QSurfaceFormat>

#include <SharedModule/internal.hpp>

class GtRendererBase : public ThreadComputingBase
{
    using Super = ThreadComputingBase;
public:
    GtRendererBase(const QSurfaceFormat& format, GtRendererBase* sharedRenderer);
    ~GtRendererBase();

private:
    void run() override;
    void compute() override;

protected:
    virtual void onInitialize() = 0;
    virtual void onDraw() = 0;
    virtual void onDestroy() = 0;

protected:
    static QMutex& initializationMutex();
    QSurfaceFormat m_surfaceFormat;
    ScopedPointer<QOpenGLContext> m_context;
    ScopedPointer<class QOffscreenSurface> m_surface;
    std::atomic_bool m_isInitialized;
    GtRendererBase* m_shareRenderer;
};

#endif // GTRENDERERBASE_H
