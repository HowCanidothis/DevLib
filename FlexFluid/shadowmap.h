#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "GraphicsTools/gt_decl.h"

struct ShadowMap
{
    GLuint texture;
    GLuint framebuffer;
    GLuint shadowProgram = 0;
    static GLuint kShadowResolution() { static GLuint res = 2048; return res; }
    static ShadowMap* ShadowCreate(OpenGLFunctions* f);

    void ShadowEnd(OpenGLFunctions* f, quint32 msaaFbo);
    void ShadowBegin(OpenGLFunctions* f);
    void ShadowApply(OpenGLFunctions* f, GLint sprogram, Vector3F& lightPos, const Vector3F& lightTarget, const Matrix4& lightTransform);
};

#endif // SHADOWMAP_H
