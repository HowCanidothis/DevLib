#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QFontMetrics>
#include <QDesktopServices>
#include <QApplication>

#include "notifywidget.h"
#include "notifyextendeddialog.h"
#include "Ui/Managers/notifymanager.h"

static QString typeToText(qint32 type)
{
    switch (type) {
    case NotifyManager::Error: return "error";
    case NotifyManager::Warning: return "warning";
    case NotifyManager::Info: return "info";
    default: return "unknown";
    }
}

NotifyWidget::NotifyWidget (NotifyData* data, int displayTime, QWidget* parent)
    : QLabel(parent)
    , m_displayTime(displayTime)
    , m_data(data)
{

    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint| Qt::Tool | Qt::WindowStaysOnTopHint);

    QString typeStr = typeToText(data->Type);

    setObjectName(typeStr + "notify-background");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    m_bodyLabel = new QLabel(this);
    m_bodyLabel->setObjectName(typeStr + "notify-body");
    m_bodyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_bodyLabel->setTextFormat(Qt::RichText);
    m_bodyLabel->setWordWrap(true);

    mainLayout->addWidget(m_iconLabel);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_bodyLabel, 1);

    m_closeBtn = new QPushButton("X", this);
    m_closeBtn->setObjectName("notify-close-btn");
    connect(m_closeBtn, &QPushButton::clicked, this, [this]{
        Q_EMIT disappeared();
    });
}


void NotifyWidget::ShowGriant(qint32 maxHeight)
{
    QPixmap tempPix = QPixmap(m_data->Icon);
    if(!tempPix.isNull()) {
        tempPix = tempPix.scaled(QSize(30, 30), Qt::KeepAspectRatio);
    }
    m_iconLabel->setPixmap(tempPix);
    m_closeBtn->move(this->width() - m_closeBtn->width(), 0);
    m_bodyLabel->setText(m_data->Body);

    if(m_bodyLabel->heightForWidth(width()) > maxHeight) {
        auto fontMetrics = m_bodyLabel->fontMetrics();
        auto elidedText = fontMetrics.elidedText(m_data->Body, Qt::ElideRight, m_bodyLabel->width() * 20);
        m_bodyLabel->setText(elidedText);
    }

    QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->setDuration(200);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [animation, this](){
        animation->deleteLater();
        QTimer::singleShot(m_displayTime, this, [this](){
            this->hideGriant();
        });
    });

    this->show();
}

void NotifyWidget::hideGriant()
{
    QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setStartValue(this->windowOpacity());
    animation->setEndValue(0);
    animation->setDuration(200);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [animation, this](){
        this->hide();
        animation->deleteLater();
        Q_EMIT disappeared();
    });
}

void NotifyWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        if(!m_data->Url.isEmpty()){
            QDesktopServices::openUrl(m_data->Url);
        }
        if(!m_data->ExtendedBody.isEmpty()) {
            NotifyExtendedDialog dialog(reinterpret_cast<QWidget*>(qApp->property("MainWindow").toLongLong()));
            dialog.SetMessage(m_data->ExtendedBody);
            dialog.exec();
        }
    }
}
