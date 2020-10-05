#include "widgethelpers.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

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

void WidgetContent::ForeachChildWidget(QWidget* target, const std::function<void (QWidget*)>& handler)
{
    auto childWidgets = target->findChildren<QWidget*>();
    for(auto* childWidget : childWidgets) {
        handler(childWidget);
    }
}
