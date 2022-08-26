#ifndef TOOLTIPWIDGET_H
#define TOOLTIPWIDGET_H

#include <QFrame>

#include <PropertiesModule/internal.hpp>

class ToolTipWidget : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
public:
    ToolTipWidget(QWidget* parent);
    ~ToolTipWidget();

    void SetContent(QWidget* content, bool deletePrevious = true);
    virtual void SetTarget(const QPoint& target);

    LocalProperty<QPoint> OffsetFromTarget;

private:
    void updateLocation();
    void updateGeometry(const QRect& rect);

private:
    QWidget* m_content;
    QPoint m_target;
    std::unique_ptr<class QPropertyAnimation, std::function<void(QPropertyAnimation*)>> m_animation;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // TOOLTIPWIDGET_H
