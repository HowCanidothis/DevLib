#ifndef TOOLTIPARROWWIDGET_H
#define TOOLTIPARROWWIDGET_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

class ToolTipArrowWidget : public QWidget
{
    using Super = QWidget;
public:
    ToolTipArrowWidget(QWidget* targetWidget);

    void SetTarget(const QPoint& target, bool update = true);

    LocalProperty<qint32> Size;

    // QWidget interface
protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updateLocation();

private:
    QPolygon m_polygon;
    QWidget* m_targetWidget;
    QPoint m_target;
};

#endif // TOOLTIPARROWWIDGET_H
