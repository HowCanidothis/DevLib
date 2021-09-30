#include "notifywidget.h"
#include "ui_notifywidget.h"

#include <QDesktopServices>
#include <QApplication>
#include <QPropertyAnimation>

#include "WidgetsModule/Utils/widgethelpers.h"

#include "notifymanager.h"

static QString typeToText(qint32 type)
{
    switch (type) {
    case NotifyManager::Error: return "error";
    case NotifyManager::Warning: return "warning";
    case NotifyManager::Info: return "info";
    default: return "unknown";
    }
}

NotifyWidget::NotifyWidget(const NotifyDataPtr& data, QWidget *parent)
    : Super(parent)
    , ui(new Ui::NotifyWidget)
    , m_data(data)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint| Qt::Tool | Qt::WindowStaysOnTopHint);

    QString typeStr = typeToText(data->Type);

    setObjectName(typeStr + "notify-background");
    ui->Body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->Body->setTextFormat(Qt::RichText);
    ui->Body->setWordWrap(true);
    ui->Body->setText(data->Body);

    connect(ui->Body, &QLabel::linkActivated, [this](const QString& link){
        bool accepted = false;
        OnLinkActivated(link, accepted);
        if(!accepted) {
            QDesktopServices::openUrl(QUrl(link));
            OnDisappeared();
        }
    });

    WidgetsAttachment::Attach(this, [this](QObject*, QEvent* event){
        if(event->type() == QEvent::Show) {
            adjustSize();
        }
        return false;
    });
}

NotifyWidget::~NotifyWidget()
{
    delete ui;
}

void NotifyWidget::ShowGriant(qint32 displayTime, const IconsSvgIcon& icon)
{
    ui->BtnIcon->setIcon(icon);
    QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->setDuration(200);
    animation->start();

    connect(animation, &QPropertyAnimation::finished, this, [animation, this, displayTime](){
        animation->deleteLater();
        QTimer::singleShot(displayTime, this, [this](){
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
        OnDisappeared();
    });
}
