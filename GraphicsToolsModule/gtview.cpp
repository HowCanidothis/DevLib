#include "gtview.h"

#include <QPainter>
#include <QResizeEvent>

#include "gtrenderer.h"

GtView::GtView(QWidget* parent, Qt::WindowFlags flags)
    : Super(parent, flags)
    , m_renderer(nullptr)
{
    setMouseTracking(true);
}


void GtView::paintEvent(QPaintEvent* )
{
    QPainter painter(this);
    painter.drawImage(rect(), m_renderer->CurrentImage());
}

GtView::~GtView()
{

}

void GtView::SetRenderer(GtRenderer* renderer)
{
    Q_ASSERT(m_renderer == nullptr);
    m_renderer = renderer;
    connect(m_renderer, SIGNAL(imageUpdated()), this, SLOT(update()));
}

void GtView::mouseMoveEvent(QMouseEvent* event)
{
    m_renderer->MouseMoveEvent(event);
}

void GtView::mousePressEvent(QMouseEvent* event)
{
    m_renderer->MousePressEvent(event);
}

void GtView::mouseReleaseEvent(QMouseEvent* event)
{
    m_renderer->MouseReleaseEvent(event);
}

void GtView::wheelEvent(QWheelEvent* event)
{
    m_renderer->WheelEvent(event);
}

void GtView::keyPressEvent(QKeyEvent *event)
{
    m_renderer->KeyPressEvent(event);
}

void GtView::resizeEvent(QResizeEvent* event)
{
    m_renderer->Resize(event->size().width(), event->size().height());
}

void GtView::keyReleaseEvent(QKeyEvent *event)
{
    m_renderer->KeyReleaseEvent(event);
}

