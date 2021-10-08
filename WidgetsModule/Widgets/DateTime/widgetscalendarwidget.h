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
    Q_PROPERTY(QColor selectedDatesBackground MEMBER m_selectedDatesBackground)
    using Super = QCalendarWidget;
public:
    WidgetsCalendarWidget(QWidget* parent = nullptr);

    LocalPropertySet<QDate> SelectedDates;

private:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;

private:
    LocalPropertyColor m_outOfRangeColor;
    LocalPropertyColor m_outOfRangeBackground;
    LocalPropertyColor m_outOfRangeBorder;
    LocalPropertyColor m_selectedDatesBackground;
};

#endif // WIDGETSCALENDARWIDGET_H
