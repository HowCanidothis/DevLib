
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QApplication>
#include <QTimer>
#include <SharedModule/internal.hpp>

#include "notifywidget.h"
#include "notifymanager.h"

NotifyManager::NotifyManager(QObject *parent)
    : QObject(parent)
    , m_enabled(true)
    , m_bottom(10)
    , m_right(10)
    , m_space(20)
    , m_width(300)
    , m_displayTime(3 * 1000)
    , m_reservedHeight(300)
    , m_freeHeight(QApplication::desktop()->availableGeometry().height() - m_reservedHeight)
    , m_maxHeight(m_freeHeight)
{
}

NotifyManager::~NotifyManager()
{
    for(auto unShowedMsgs : m_dataQueue) {
        delete  unShowedMsgs;
    }
}

void formattedText(const QString& body, QString& formattedBody, QString& formatedExtendedBody)
{
    QString fmtMsg = body;
    fmtMsg.replace("\\n", "<br/>");
    QStringList bodies = fmtMsg.split("EXTENDED_BODY:");
    formattedBody = bodies.first();
    if(bodies.size() == 2) {
        formatedExtendedBody = bodies.last();
    }
}

void NotifyManager::EnableNotifications(bool enabled)
{
    m_enabled = enabled;
}

void NotifyManager::Notify(NotifyManager::MessageType messageType, const QString& body)
{
    if(!m_enabled) {
        return;
    }
    QString formattedBody, formattedExtendedBody;
    formattedText(body, formattedBody, formattedExtendedBody);
    auto data = new NotifyData(messageType, formattedBody, formattedExtendedBody);

    m_dataQueue.enqueue(data);
    showNext();
}

void NotifyManager::Notify(QtMsgType qtMessageType, const QString& body)
{
    if(!m_enabled) {
        return;
    }
    switch (qtMessageType) {
    case QtWarningMsg: Notify(NotifyManager::Warning, body); break;
    case QtCriticalMsg: Notify(NotifyManager::Error, body); break;
    case QtInfoMsg: Notify(NotifyManager::Info, body); break;
    default: break;
    }
}

NotifyManager& NotifyManager::GetInstance()
{
    static NotifyManager manager;
    return manager;
}

void NotifyManager::SetMargins(qint32 bottom, qint32 right, qint32 space)
{
    m_bottom = bottom;
    m_right = right;
    m_space = space;
}

void NotifyManager::SetWidth(qint32 width)
{
    m_width = width;
}

void NotifyManager::SetDisplayTime(int ms)
{
    m_displayTime = ms;
}

void NotifyManager::rearrange()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->availableGeometry();
    QPoint bottomRignt = desktopRect.bottomRight();

    qint32 index = 1;
    qint32 height = 0;
    for(NotifyWidget* notifyWidget : m_notifyList) {
        height += notifyWidget->sizeHint().height() + m_space;
        QPoint pos = bottomRignt - QPoint(m_width + m_right, height + m_bottom);
        notifyWidget->setProperty("position", pos);
        QPropertyAnimation *animation = new QPropertyAnimation(notifyWidget, "pos", this);
        animation->setStartValue(notifyWidget->pos());
        animation->setEndValue(pos);
        animation->setDuration(300);
        animation->start();

        connect(animation, &QPropertyAnimation::finished, this, [animation, this](){
            animation->deleteLater();
        });
        index++;
    }

    m_freeHeight = bottomRignt.y() - height - m_reservedHeight;
}

void NotifyManager::showNext()
{
    if(m_freeHeight < 0 || m_dataQueue.isEmpty()) {
        return;
    }

    auto data = m_dataQueue.dequeue();

    NotifyWidget* notify = new NotifyWidget(data, this->m_displayTime);
    LinkActivated.ConnectFrom(notify->LinkActivated);
    notify->setFixedWidth(m_width);

    QDesktopWidget* desktop = QApplication::desktop();
    QRect desktopRect = desktop->availableGeometry();

    notify->ShowGriant(m_maxHeight);

    qint32 usedSpace = notify->height() + m_space;
    QPoint bottomRignt = desktopRect.bottomRight();
    QPoint pos = bottomRignt - QPoint(notify->width() + m_right, (bottomRignt.y() - m_freeHeight - m_reservedHeight) + usedSpace + m_bottom);

    // TODO. Due to limitations of OS and Qt. Synchronize this values with styleSheets borders
    QPainterPath path;
    path.addRoundedRect(notify->rect(), 3.0, 3.0);
    notify->setMask(path.toFillPolygon().toPolygon());

    m_freeHeight -= usedSpace;

    notify->move(pos);
    m_notifyList.append(notify);

    connect(notify, &NotifyWidget::disappeared, this, [notify, this](){
        this->m_notifyList.removeOne(notify);
        this->rearrange();
        this->showNext();

        notify->deleteLater();
    });
}

