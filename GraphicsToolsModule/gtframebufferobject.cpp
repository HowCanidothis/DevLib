#include "gtframebufferobject.h"

GtFramebufferObjectBase::~GtFramebufferObjectBase()
{
    f->glDeleteRenderbuffers(1, &m_depthRenderBuffer);
    f->glDeleteFramebuffers(1, &m_id);
}

void GtFramebufferObjectBase::BindRead()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void GtFramebufferObjectBase::Bind()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void GtFramebufferObjectBase::Release()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


GtFramebufferObject::GtFramebufferObject(OpenGLFunctions* f, const SizeI& resolution)
    : GtFramebufferObjectBase(f, resolution)
{

}

void GtFramebufferObject::Create(const GtFramebufferFormat& format)
{
    f->glGenFramebuffers(1, &m_id);
    GtFramebufferObjectBinderScopeGuard binder(this);

    const auto& color_formats = format.m_colorFormats;
    GLenum attachments = GL_COLOR_ATTACHMENT0;

    for(const GtFramebufferTextureFormat& tex_format : color_formats) {
        GtTexture* texture = GtTexture::Create(f, tex_format.m_target, tex_format.m_internalFormat, m_resolution, &tex_format);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, attachments++, texture->GetTarget(), texture->GetId(), 0);
        m_colorAttachments.Push(texture);
    }

    switch (format.m_depthAttachment) {
    case GtFramebufferFormat::RenderBuffer:{
        f->glGenRenderbuffers(1, &m_depthRenderBuffer);
        f->glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
        f->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_resolution.width(), m_resolution.height());
        f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
        break;
    }
    case GtFramebufferFormat::Texture:{
        m_depthTexture = new GtTexture2D(f);
        m_depthTexture->SetInternalFormat(GL_DEPTH_COMPONENT16);
        m_depthTexture->SetSize(m_resolution.width(), m_resolution.height());
        GtTextureFormat depth_format;
        depth_format.PixelFormat = GL_DEPTH_COMPONENT;
        depth_format.PixelType = GL_FLOAT;
        m_depthTexture->Allocate(depth_format);
        f->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture->GetId(), 0);
        break;
    }
    default:
        break;
    }


    GLuint status = f->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        qCWarning(LC_SYSTEM) << "incomplete framebuffer";
    }
}

GtFramebufferObjectMultisampled::GtFramebufferObjectMultisampled(OpenGLFunctions* f, const SizeI& resolution, quint32 samples)
    : GtFramebufferObjectBase(f, resolution)
    , m_samples(samples)
{
    Q_ASSERT(samples > 1);
}

GtFramebufferObjectMultisampled::~GtFramebufferObjectMultisampled()
{
    f->glDeleteRenderbuffers(m_colorAttachments.Size(), m_colorAttachments.data());
}

void GtFramebufferObjectMultisampled::Create(const GtFramebufferFormat& format)
{
    f->glGenFramebuffers(1, &m_id);
    GtFramebufferObjectBinderScopeGuard binder(this);

    const auto& color_formats = format.m_colorFormats;
    if(!color_formats.IsEmpty()) {
        m_colorAttachments.Resize(color_formats.Size());
        auto it_colors = m_colorAttachments.Begin();

        f->glGenRenderbuffers(color_formats.Size(), it_colors);
        GLenum attachments = GL_COLOR_ATTACHMENT0;

        for(const GtFramebufferTextureFormat& tex_format : color_formats) {
            f->glBindRenderbuffer(GL_RENDERBUFFER, *it_colors);
            f->glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, tex_format.m_internalFormat, m_resolution.width(), m_resolution.height());
            f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachments++, GL_RENDERBUFFER, *it_colors++);
        }
    }

    switch (format.m_depthAttachment) {
    case GtFramebufferFormat::RenderBuffer:{
        f->glGenRenderbuffers(1, &m_depthRenderBuffer);
        f->glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
        f->glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH_COMPONENT16, m_resolution.width(), m_resolution.height());
        f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
        break;
    }
    case GtFramebufferFormat::Texture:{
        m_depthTexture = new GtTexture2DMultisampled(f, m_samples);
        m_depthTexture->SetInternalFormat(GL_DEPTH_COMPONENT16);
        m_depthTexture->SetSize(m_resolution.width(), m_resolution.height());
        GtTextureFormat depth_format;
        depth_format.PixelFormat = GL_DEPTH_COMPONENT;
        depth_format.PixelType = GL_FLOAT;
        m_depthTexture->Allocate(depth_format);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depthTexture->GetId(), 0);
        break;
    }
    default:
        break;
    }

    GLuint status = f->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        qCWarning(LC_SYSTEM) << "incomplete framebuffer";
    }
}
