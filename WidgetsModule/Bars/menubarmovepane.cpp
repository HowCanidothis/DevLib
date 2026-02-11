#include "menubarmovepane.h"
#include "ui_menubarmovepane.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#include <windowsx.h>
#endif

#include <QMouseEvent>
#include <QGraphicsEffect>

#include <WidgetsModule/internal.hpp>

MenuBarMovePane::MenuBarMovePane(QWidget* parent)
    : Super(parent)
    , Closeable(true)
    , Resizeable(true)
    , Modal(false)
    , ui(new Ui::MenuBarMovePane())
    , m_windowEventFilter(nullptr)
    , m_ignoreMoveEvents(false)
    , m_isDrag(false)
{
    ui->setupUi(this);
    ui->Title->setText(parent->windowTitle());
    ui->Title->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->BtnClose->setAttribute(Qt::WA_NoMousePropagation);
    ui->BtnMinimize->setAttribute(Qt::WA_NoMousePropagation);
    ui->BtnMaximize->setAttribute(Qt::WA_NoMousePropagation);

    SetWindow(this);

    Resizeable.Subscribe([this]{
        ui->BtnMaximize->setVisible(Resizeable);
    });

    Closeable.Subscribe([this]{
        ui->BtnClose->setVisible(Closeable);
    });

    Modal.Subscribe([this]{
        ui->BtnMinimize->setVisible(!Modal);
    });
    m_windowGeometry = parent->window()->saveGeometry();
}

bool MenuBarMovePane::filter(QObject* watched, QEvent* event)
{
    switch(event->type()) {
    case QEvent::WindowTitleChange: {
        auto* widget = reinterpret_cast<QWidget*>(watched);
        SetTitle(widget->windowTitle());
    } break;
    case QEvent::Resize:
    case QEvent::Move: {
        auto* widget = window();
        if(!widget->isMaximized()) {
            m_windowGeometry = window()->saveGeometry();
        }
    } break;
    case QEvent::WindowStateChange: {
        auto* widget = window();
        auto* ce = static_cast<QWindowStateChangeEvent*>(event);
        auto dif = ce->oldState() ^ widget->windowState();
        if(dif & Qt::WindowMaximized && widget->windowState() & Qt::WindowMaximized) {
            WidgetWrapper(ui->BtnMaximize).ApplyStyleProperty("windowMaximized", true);
            window()->layout()->setMargin(0);
        } else {
            WidgetWrapper(ui->BtnMaximize).ApplyStyleProperty("windowMaximized", false);
            window()->layout()->setMargin(10);
        }
    } break;
    default: break;
    }

    return false;
}

void MenuBarMovePane::SetWindow(QWidget* w)
{
    if(m_windowEventFilter != nullptr) {
        m_windowEventFilter->deleteLater();
    }
    m_windowEventFilter = WidgetWrapper(w->window()).AddEventFilter([this](QObject* o, QEvent* e){ return filter(o,e); });
}

QLabel* MenuBarMovePane::GetTitleWidget() const
{
    return ui->Title;
}

QHBoxLayout* MenuBarMovePane::GetPlaceHolder() const
{
    return ui->PlaceHolder;
}

QString MenuBarMovePane::GetTitle() const
{
    return ui->Title->text();
}

void MenuBarMovePane::SetTitle(const QString& title)
{
    ui->Title->setText(title);
}

void MenuBarMovePane::mousePressEvent(QMouseEvent* event)
{
    Super::mousePressEvent(event);
    m_pressPoint = event->pos();
    m_ignoreMoveEvents = false;
}

void MenuBarMovePane::mouseMoveEvent(QMouseEvent* event)
{
    Super::mouseMoveEvent(event);
    if(!m_ignoreMoveEvents && event->pos() != m_pressPoint && event->buttons() == Qt::LeftButton) {
        if(window()->isMaximized()) {
            window()->layout()->setMargin(10);
            WidgetWrapper(window()).RestoreGeometry(m_windowGeometry);
            OnGeometryRestored(window());
            auto size = window()->size();
            auto pos = cursor().pos();
            window()->move(pos.x() - size.width() / 2, pos.y() - 10);
            m_pressPoint = mapFromGlobal(pos);
        } else {
            auto distance = event->pos() - m_pressPoint;
            window()->move(window()->pos() + distance);
        }

#ifdef Q_OS_WIN
        if (!m_isDrag){
            m_isDrag = true;
            ::SendNotifyMessage(HWND(window()->winId()), WM_ENTERSIZEMOVE, SC_MOVE, NULL);
        }

        auto rect = window()->geometry();
        RECT rectW {rect.left(), rect.top(), rect.right(), rect.bottom()};
        ::SendMessageA(HWND(window()->winId()), WM_MOVING, NULL, LPARAM(&rectW));
#endif
    }
}

void MenuBarMovePane::mouseDoubleClickEvent(QMouseEvent* event)
{
    Super::mouseDoubleClickEvent(event);
    m_ignoreMoveEvents = true;
    if(event->button() == Qt::LeftButton) {
        MaximizeRestore();
    }
}

void MenuBarMovePane::MaximizeRestore()
{
    if(!Resizeable) {
        return;
    }    
    if(window()->isMaximized()) {
        WidgetWrapper(window()).RestoreGeometry(m_windowGeometry);
        OnGeometryRestored(window());
    } else {
        window()->showMaximized();
    }
}

void MenuBarMovePane::mouseReleaseEvent(QMouseEvent* event)
{
    Super::mouseReleaseEvent(event);
#ifdef Q_OS_WIN
    if (m_isDrag) {
        ::SendNotifyMessage(HWND(window()->winId()), WM_EXITSIZEMOVE, SC_MOVE, NULL);
        m_isDrag = false;
    }
#endif
    m_ignoreMoveEvents = true;
}

void MenuBarMovePane::on_BtnMinimize_clicked()
{
    window()->showMinimized();
}

void MenuBarMovePane::on_BtnMaximize_clicked()
{
    MaximizeRestore();
}

void MenuBarMovePane::on_BtnClose_clicked()
{
    window()->close();
}
