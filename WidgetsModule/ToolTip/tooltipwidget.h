#ifndef TOOLTIPWIDGET_H
#define TOOLTIPWIDGET_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

class ToolTipWidget : public QFrame
{
    using Super = QFrame;
public:
    ToolTipWidget(QWidget* parent);
    ~ToolTipWidget();

    virtual void SetTarget(const QPoint& target);

    LocalProperty<QPoint> OffsetFromTarget;

private:
    void updateLocation();
    void updateGeometry(const QRect& rect);

private:
    QPoint m_target;
    ScopedPointer<class QPropertyAnimation> m_animation;
};

#endif // TOOLTIPWIDGET_H
