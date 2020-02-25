#ifndef GTDEPTHBUFFER_H
#define GTDEPTHBUFFER_H

#include <SharedGuiModule/internal.hpp>

class GtDepthBuffer
{
public:
    GtDepthBuffer(OpenGLFunctions* f);

    void SetFrameBuffer(class GtFramebufferObjectBase* frameBuffer);

    float ValueAt(qint32 x, qint32 y);

private:
    OpenGLFunctions* m_f;
    class GtFramebufferObjectBase* m_frameBuffer;
};

#endif // GTDEPTHBUFFER_H
