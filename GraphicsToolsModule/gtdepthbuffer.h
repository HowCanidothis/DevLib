#ifndef GTDEPTHBUFFER_H
#define GTDEPTHBUFFER_H

#include <SharedGuiModule/internal.hpp>

class GtDepthBuffer
{
public:
    GtDepthBuffer(OpenGLFunctions* f);

    void SetFrameBuffer(class GtFramebufferObjectBase* frameBuffer, QOpenGLContext* context);

    float ValueAt(qint32 x, qint32 y);

private:
    OpenGLFunctions* m_f;
    QOpenGLContext* m_context;
    class GtFramebufferObjectBase* m_frameBuffer;
};

#endif // GTDEPTHBUFFER_H
