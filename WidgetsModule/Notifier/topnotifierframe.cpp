#include "topnotifierframe.h"
#include "ui_topnotifierframe.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QResizeEvent>


void WidgetAppearance::SetVisibleAnimated(QWidget* widget, bool visible)
{
    if(visible) {
        WidgetAppearance::ShowAnimated(widget);
    } else {
        WidgetAppearance::HideAnimated(widget);
    }
}

void WidgetAppearance::ShowAnimated(QWidget* widget)
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect,"opacity");
    animation->setDuration(2000);
    animation->setStartValue(0.0);
    animation->setEndValue(0.8);
    animation->setEasingCurve(QEasingCurve::InBack);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
    widget->show();
}

void WidgetAppearance::HideAnimated(QWidget* widget)
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect,"opacity");
    animation->setDuration(2000);
    animation->setStartValue(0.8);
    animation->setEndValue(0);
    animation->setEasingCurve(QEasingCurve::OutBack);
    animation->connect(animation, &QPropertyAnimation::finished, [widget]{
        widget->hide();
    });
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

TopNotifierFrame::TopNotifierFrame(QWidget* parent)
    : Super(parent)
    , ui(new Ui::TopNotifierFrame)
{
    ui->setupUi(this);
    ui->BtnAction->setVisible(false);

    parent->installEventFilter(this);
    hide();
}

TopNotifierFrame::~TopNotifierFrame()
{
    delete ui;
}

void TopNotifierFrame::SetText(const QString& text)
{
    ui->Label->setText(text);
}

void TopNotifierFrame::SetActionText(const QString& text)
{
    ui->BtnAction->setText(text);
    ui->BtnAction->setVisible(!text.isEmpty());
}

void TopNotifierFrame::SetAction(const FAction& action)
{
    connect(ui->BtnAction, &QPushButton::clicked, action);
}

bool TopNotifierFrame::eventFilter(QObject*, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Resize: {
        auto* resizeEvent = reinterpret_cast<QResizeEvent*>(event);
        move((resizeEvent->size().width() - width()) / 2, 0);
        break;
    }
    default: break;
    }

    return false;
}
