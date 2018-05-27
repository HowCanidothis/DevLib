#include "gtframebufferobject.h"

GtFramebufferObjectBase::~GtFramebufferObjectBase()
{
    f->glDeleteRenderbuffers(1, &depth_render_buffer);
    f->glDeleteFramebuffers(1, &id);
}

void GtFramebufferObjectBase::bind()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void GtFramebufferObjectBase::release()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


GtFramebufferObject::GtFramebufferObject(OpenGLFunctions* f, const SizeI& resolution)
    : GtFramebufferObjectBase(f, resolution)
{

}

void GtFramebufferObject::create(const GtFramebufferFormat& format)
{
    LOGOUT;

    f->glGenFramebuffers(1, &id);
    GtFramebufferObjectBinder binder(this);

    const auto& color_formats = format.color_formats;
    GLenum attachments = GL_COLOR_ATTACHMENT0;

    for(const GtFramebufferTextureFormat& tex_format : color_formats) {
        GtTexture* texture = GtTexture::create(f, tex_format.target, tex_format.internal_format, resolution, &tex_format);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, attachments++, texture->getTarget(), texture->getID(), 0);
        color_attachments.Push(texture);
    }

    switch (format.depth_attachment) {
    case GtFramebufferFormat::RenderBuffer:{
        f->glGenRenderbuffers(1, &depth_render_buffer);
        f->glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
        f->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, resolution.width(), resolution.height());
        f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);
        break;
    }
    case GtFramebufferFormat::Texture:{
        depth_texture = new GtTexture2D(f);
        depth_texture->setInternalFormat(GL_DEPTH_COMPONENT16);
        depth_texture->setSize(resolution.width(), resolution.height());
        GtTextureFormat depth_format;
        depth_format.pixels_format = GL_DEPTH_COMPONENT;
        depth_format.pixels_type = GL_FLOAT;
        depth_texture->allocate(depth_format);
        f->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture->getID(), 0);
        break;
    }
    default:
        break;
    }


    GLuint status = f->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        log.Warning() << "incomplete framebuffer";
    }
}

GtFramebufferObjectMultisampled::GtFramebufferObjectMultisampled(OpenGLFunctions* f, const SizeI& resolution, quint32 samples)
    : GtFramebufferObjectBase(f, resolution)
    , samples(samples)
{
    Q_ASSERT(samples > 1);
}

GtFramebufferObjectMultisampled::~GtFramebufferObjectMultisampled()
{
    f->glDeleteRenderbuffers(color_attachments.Size(), color_attachments.data());
}

void GtFramebufferObjectMultisampled::create(const GtFramebufferFormat& format)
{
    LOGOUT;

    f->glGenFramebuffers(1, &id);
    GtFramebufferObjectBinder binder(this);

    const auto& color_formats = format.color_formats;
    if(!color_formats.IsEmpty()) {
        color_attachments.Resize(color_formats.Size());
        auto it_colors = color_attachments.Begin();

        f->glGenRenderbuffers(color_formats.Size(), it_colors);
        GLenum attachments = GL_COLOR_ATTACHMENT0;

        for(const GtFramebufferTextureFormat& tex_format : color_formats) {
            f->glBindRenderbuffer(GL_RENDERBUFFER, *it_colors);
            f->glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, tex_format.internal_format, resolution.width(), resolution.height());
            f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachments++, GL_RENDERBUFFER, *it_colors++);
        }
    }

    switch (format.depth_attachment) {
    case GtFramebufferFormat::RenderBuffer:{
        f->glGenRenderbuffers(1, &depth_render_buffer);
        f->glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
        f->glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT16, resolution.width(), resolution.height());
        f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);
        break;
    }
    case GtFramebufferFormat::Texture:{
        depth_texture = new GtTexture2DMultisampled(f, samples);
        depth_texture->setInternalFormat(GL_DEPTH_COMPONENT16);
        depth_texture->setSize(resolution.width(), resolution.height());
        GtTextureFormat depth_format;
        depth_format.pixels_format = GL_DEPTH_COMPONENT;
        depth_format.pixels_type = GL_FLOAT;
        depth_texture->allocate(depth_format);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth_texture->getID(), 0);
        break;
    }
    default:
        break;
    }

    GLuint status = f->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        log.Warning() << "incomplete framebuffer";
    }
}
