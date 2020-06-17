#ifndef GTRENDERERBASE_H
#define GTRENDERERBASE_H

#include <QSurfaceFormat>

#include <SharedModule/internal.hpp>

class GtRendererBase : public ThreadComputingBase
{
    using Super = ThreadComputingBase;
public:
    GtRendererBase();
    ~GtRendererBase();

    void SetFormat(const QSurfaceFormat& format);
    void Resize(qint32 w, qint32 h);

private:
    void run() override;
    void compute() override;

protected:
    virtual void onInitialize() = 0;
    virtual void onResize(qint32 w, qint32 h) = 0;
    virtual void onDraw() = 0;
    virtual void onDestroy() = 0;

protected:
    static QMutex& initializationMutex();
    QSurfaceFormat m_surfaceFormat;
    ScopedPointer<QOpenGLContext> m_context;
    ScopedPointer<class QOffscreenSurface> m_surface;
    bool m_isInitialized;
};

#endif // GTRENDERERBASE_H
