#ifndef WIDGETSCALENDARWIDGET_H
#define WIDGETSCALENDARWIDGET_H

#include <QCalendarWidget>

#include <PropertiesModule/internal.hpp>

// TODO. Replace this component with a custom one to avoid using this piece of sheet
class WidgetsCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor outOfRangeColor MEMBER m_outOfRangeColor)
    Q_PROPERTY(QColor outOfRangeBackground MEMBER m_outOfRangeBackground)
    Q_PROPERTY(QColor outOfRangeBorder MEMBER m_outOfRangeBorder)
    using Super = QCalendarWidget;
public:
    WidgetsCalendarWidget(QWidget* parent = nullptr);

private:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;

private:
    LocalPropertyColor m_outOfRangeColor;
    LocalPropertyColor m_outOfRangeBackground;
    LocalPropertyColor m_outOfRangeBorder;
};

#endif // WIDGETSCALENDARWIDGET_H
