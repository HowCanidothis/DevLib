#include "widgetscalendarwidget.h"

#include <QTextCharFormat>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsCalendarWidget::WidgetsCalendarWidget(QWidget* parent)
    : Super(parent)
    , m_outOfRangeColor(QColor(Qt::transparent))
    , m_outOfRangeBackground(QColor(Qt::transparent))
    , m_outOfRangeBorder(QColor(Qt::transparent))
{
    auto updateHandler = [this]{
        update();
    };
    m_outOfRangeColor.Subscribe(updateHandler);
    m_outOfRangeBackground.Subscribe(updateHandler);
    m_outOfRangeBorder.Subscribe(updateHandler);
    m_selectedDatesBackground.Subscribe(updateHandler);
    SelectedDates.Subscribe(updateHandler);

    QTextCharFormat fmt;
#ifndef QT_PLUGIN
    fmt.setForeground(QBrush(SharedSettings::GetInstance().StyleSettings.DefaultRedColor));
#endif
    setHeaderTextFormat(fmt);
    setVerticalHeaderFormat(NoVerticalHeader);
}

void WidgetsCalendarWidget::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
{
    if(m_outOfRangeBackground == Qt::transparent && m_outOfRangeColor == Qt::transparent && m_outOfRangeBorder == Qt::transparent) {
        Super::paintCell(painter, rect, date);
        return;
    }

    auto backgroundColor = m_outOfRangeBackground.Native();

    if(date != selectedDate() && SelectedDates.IsContains(date)) {
        backgroundColor = m_selectedDatesBackground.Native();
    }

    if (date < minimumDate() || date > maximumDate() || backgroundColor != m_outOfRangeBackground) {
        painter->save();
        auto penColor = painter->pen().color();
        painter->setPen(m_outOfRangeBorder);
        painter->setBrush(QBrush(backgroundColor));
        painter->drawRect(rect);
        if(m_outOfRangeColor != Qt::transparent) {
            painter->setPen(m_outOfRangeColor);
        } else {
            painter->setPen(penColor);
        }
        painter->drawText(rect, Qt::TextSingleLine | Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    } else {
        Super::paintCell(painter, rect, date);
    }
}
