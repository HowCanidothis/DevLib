#include "gtview.h"

#include <QPainter>
#include <QResizeEvent>

#include "gtrenderercontroller.h"

#include <private/qhighdpiscaling_p.h>

GtView::GtView(QWidget* parent, Qt::WindowFlags flags)
    : Super(parent, flags)
    , m_controller(nullptr)
{
    setMouseTracking(true);
}


void GtView::paintEvent(QPaintEvent* )
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawRect(rect());
    painter.drawImage(rect(), m_controller->GetCurrentImage());
#ifdef QT_DEBUG
    painter.drawText(QRect(0,0,100,200), QString::number(1000000000.0 / m_controller->GetRenderTime()));
#endif
}

GtView::~GtView()
{

}

void GtView::SetController(const GtRendererControllerPtr& controller)
{
    Q_ASSERT(m_controller == nullptr);
    m_controller = controller;
    connect(m_controller.get(), SIGNAL(imageUpdated()), this, SLOT(update()));
}

void GtView::mouseMoveEvent(QMouseEvent* event)
{
    if(screen() == nullptr) {
        return;
    }
    QMouseEvent e(event->type(), QHighDpi::toNativePixels(event->pos(), screen()), event->button(), event->buttons(), event->modifiers());
    m_clicked = false;
    if(!hasFocus()) {
        setFocus();
    }
    m_controller->MouseMoveEvent(&e);
}

void GtView::mousePressEvent(QMouseEvent* event)
{
    if(screen() == nullptr) {
        return;
    }
    QMouseEvent e(event->type(), QHighDpi::toNativePixels(event->pos(), screen()), event->button(), event->buttons(), event->modifiers());
    m_clicked = true;
    m_controller->MousePressEvent(&e);
}

void GtView::mouseReleaseEvent(QMouseEvent* event)
{
    if(screen() == nullptr) {
        return;
    }
    QMouseEvent e(event->type(), QHighDpi::toNativePixels(event->pos(), screen()), event->button(), event->buttons(), event->modifiers());

    if(m_clicked) {
        m_controller->MouseClickedEvent(&e);
    } else {
        m_controller->MouseReleaseEvent(&e);
    }
}

void GtView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(screen() == nullptr) {
        return;
    }
    QMouseEvent e(event->type(), QHighDpi::toNativePixels(event->pos(), screen()), event->button(), event->buttons(), event->modifiers());
    m_controller->MouseDoubleClickEvent(&e);
}

void GtView::wheelEvent(QWheelEvent* event)
{
    if(screen() == nullptr) {
        return;
    }
    QWheelEvent e(QHighDpi::toNativePixels(event->position(), screen()), event->delta(), event->buttons(), event->modifiers(), event->orientation());
    m_controller->WheelEvent(&e);
}

void GtView::keyPressEvent(QKeyEvent *event)
{
    m_controller->KeyPressEvent(event);
}

void GtView::resizeEvent(QResizeEvent* event)
{
    auto* screen = this->screen();
    if(screen == nullptr) {
        return;
    }
    auto scale = QHighDpiScaling::scaleAndOrigin(screen).factor;
    m_controller->SetScaleFactor(scale);
    auto size = QHighDpi::toNativePixels(event->size(), screen);
    m_controller->Resize(size.width(), size.height());
}

void GtView::keyReleaseEvent(QKeyEvent *event)
{
    m_controller->KeyReleaseEvent(event);
}

void GtView::leaveEvent(QEvent *event)
{
    m_controller->LeaveEvent(event);
}

