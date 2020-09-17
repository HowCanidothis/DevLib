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
    std::unique_ptr<class QPropertyAnimation, std::function<void(QPropertyAnimation*)>> m_animation;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // TOOLTIPWIDGET_H
