#ifndef GTFRAMEBUFFEROBJECT_H
#define GTFRAMEBUFFEROBJECT_H

#include "gttexture2D.h"
#include "SharedModule/stack.h"

class GtFramebufferTextureFormat : GtTextureFormat
{
public:
    GtFramebufferTextureFormat(gTexTarget target, gTexInternalFormat internal_format)
        : target(target)
        , internal_format(internal_format)
    {}
private:
    friend class GtFramebufferObject;
    friend class GtFramebufferObjectMultisampled;
    gTexTarget target;
    gTexInternalFormat internal_format;
};
Q_DECLARE_TYPEINFO(GtFramebufferTextureFormat, Q_PRIMITIVE_TYPE);

class GtFramebufferFormat
{
public:
    enum AttachmentType {
        NoAttachment,
        RenderBuffer,
        Texture
    };

    void setDepthAttachment(AttachmentType type) { depth_attachment = type; }
    void addColorAttachment(const GtFramebufferTextureFormat& format) { color_formats.Push(format); }
private:
    friend class GtFramebufferObject;
    friend class GtFramebufferObjectMultisampled;
    Stack<GtFramebufferTextureFormat> color_formats;
    AttachmentType depth_attachment;
};

class GtFramebufferObjectBase
{
public:
    GtFramebufferObjectBase(OpenGLFunctions* f, const SizeI& resolution)
        : f(f)
        , id(0)
        , depth_texture(nullptr)
        , depth_render_buffer(0)
        , resolution(resolution)
    {}
    virtual ~GtFramebufferObjectBase();

    void bind();
    void release();

    quint32 getWidth() const { return resolution.width(); }
    quint32 getHeight() const { return resolution.height(); }

    const GtTexture* getDepthTexture() const { return depth_texture.data(); }
    gRenderbufferID getDepthRenderbuffer() const { return depth_render_buffer; }
    gFboID getID() const { return id; }
protected:
    OpenGLFunctions* f;
    gFboID id;
    ScopedPointer<GtTexture> depth_texture;
    gRenderbufferID depth_render_buffer;
    SizeI resolution;
};

class GtFramebufferObject : public GtFramebufferObjectBase
{
public:
    GtFramebufferObject(OpenGLFunctions* f, const SizeI& );

    void create(const GtFramebufferFormat& format);

    GtTexture* getColorTexture(qint32 index) const { return color_attachments.At(index); }
private:
    StackPointers<GtTexture> color_attachments;
};

class GtFramebufferObjectMultisampled : public GtFramebufferObjectBase
{
public:
    GtFramebufferObjectMultisampled(OpenGLFunctions* f, const SizeI& , quint32 samples);
    ~GtFramebufferObjectMultisampled();

    void create(const GtFramebufferFormat& format);

    gRenderbufferID getColorRenderbuffer(qint32 index) const { return color_attachments.At(index); }
private:
    Stack<gRenderbufferID> color_attachments;
    qint32 samples;
};

struct GtFramebufferObjectBinder
{
    GtFramebufferObjectBase* fbo;
public:
    GtFramebufferObjectBinder(GtFramebufferObjectBase* frame_buffer) NO_EXCEPT
        : fbo(frame_buffer)
    {
        fbo->bind();
    }
    ~GtFramebufferObjectBinder()
    {
        fbo->release();
    }
};

#endif // GTFRAMEBUFFEROBJECT_H
