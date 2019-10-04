#ifndef GTFRAMEBUFFEROBJECT_H
#define GTFRAMEBUFFEROBJECT_H

#include "gttexture2D.h"
#include "SharedModule/stack.h"

class GtFramebufferTextureFormat : GtTextureFormat
{
public:
    GtFramebufferTextureFormat(gTexTarget target, gTexInternalFormat internal_format)
        : m_target(target)
        , m_internalFormat(internal_format)
    {}

private:
    friend class GtFramebufferObject;
    friend class GtFramebufferObjectMultisampled;
    gTexTarget m_target;
    gTexInternalFormat m_internalFormat;
};
Q_DECLARE_TYPEINFO(GtFramebufferTextureFormat, Q_PRIMITIVE_TYPE);

namespace std
{
    template<>
    struct is_pod<GtFramebufferTextureFormat> : bool_constant<true>
    {};
};

class GtFramebufferFormat
{
public:
    enum AttachmentType {
        NoAttachment,
        RenderBuffer,
        Texture
    };

    void SetDepthAttachment(AttachmentType type) { m_depthAttachment = type; }
    void AddColorAttachment(const GtFramebufferTextureFormat& format) { m_colorFormats.Push(format); }

private:
    friend class GtFramebufferObject;
    friend class GtFramebufferObjectMultisampled;
    Stack<GtFramebufferTextureFormat> m_colorFormats;
    AttachmentType m_depthAttachment;
};

class GtFramebufferObjectBase
{
public:
    GtFramebufferObjectBase(OpenGLFunctions* f, const SizeI& resolution)
        : f(f)
        , m_id(0)
        , m_depthTexture(nullptr)
        , m_depthRenderBuffer(0)
        , m_resolution(resolution)
    {}
    virtual ~GtFramebufferObjectBase();

    void Bind();
    void Release();

    quint32 GetWidth() const { return m_resolution.width(); }
    quint32 GetHeight() const { return m_resolution.height(); }

    const GtTexture* GetDepthTexture() const { return m_depthTexture.data(); }
    gRenderbufferID GetDepthRenderbuffer() const { return m_depthRenderBuffer; }
    gFboID GetID() const { return m_id; }

protected:
    OpenGLFunctions* f;
    gFboID m_id;
    ScopedPointer<GtTexture> m_depthTexture;
    gRenderbufferID m_depthRenderBuffer;
    SizeI m_resolution;
};

class GtFramebufferObject : public GtFramebufferObjectBase
{
public:
    GtFramebufferObject(OpenGLFunctions* f, const SizeI& );

    void Create(const GtFramebufferFormat& format);
    GtTexture* GetColorTexture(qint32 index) const { return m_colorAttachments.At(index); }

private:
    StackPointers<GtTexture> m_colorAttachments;
};

class GtFramebufferObjectMultisampled : public GtFramebufferObjectBase
{
public:
    GtFramebufferObjectMultisampled(OpenGLFunctions* f, const SizeI& , quint32 m_samples);
    ~GtFramebufferObjectMultisampled();

    void Create(const GtFramebufferFormat& format);
    gRenderbufferID GetColorRenderbuffer(qint32 index) const { return m_colorAttachments.At(index); }

private:
    Stack<gRenderbufferID> m_colorAttachments;
    qint32 m_samples;
};

struct GtFramebufferObjectBinderScopeGuard
{
    GtFramebufferObjectBase* m_fbo;
public:
    GtFramebufferObjectBinderScopeGuard(GtFramebufferObjectBase* frame_buffer) Q_DECL_NOEXCEPT
        : m_fbo(frame_buffer)
    {
        m_fbo->Bind();
    }
    ~GtFramebufferObjectBinderScopeGuard()
    {
        m_fbo->Release();
    }
};

#endif // GTFRAMEBUFFEROBJECT_H
