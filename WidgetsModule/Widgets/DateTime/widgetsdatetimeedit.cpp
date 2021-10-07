#include "widgetsdatetimeedit.h"

#include <QKeyEvent>
#include <QCalendarWidget>

#include "WidgetsModule/Utils/styleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDateTimeEdit::WidgetsDateTimeEdit(QWidget* parent)
    : Super(parent)
{
    init();
}

WidgetsDateTimeEdit::WidgetsDateTimeEdit(const QVariant& date, QVariant::Type type, QWidget* parent)
    : Super(date, type, parent)
{
    init();
}

QDateTime WidgetsDateTimeEdit::dateTimeFromText(const QString& text) const
{
    return !CurrentDateTime.IsRealTime() ? Super::dateTimeFromText(text) : QDateTime();
}

QString WidgetsDateTimeEdit::textFromDateTime(const QDateTime& dt) const
{
    return !CurrentDateTime.IsRealTime() ? Super::textFromDateTime(dt) : "";
}

void WidgetsDateTimeEdit::init()
{
    m_recursionBlock = false;
    auto update = [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::LambdaGuard guard([this]{ m_recursionBlock = false; }, [this]{ m_recursionBlock = true; });
        setReadOnly(CurrentDateTime.IsRealTime());

        setDisplayFormat(displayFormat());
        setDateTime(CurrentDateTime);
        StyleUtils::UpdateStyle(this);
    };
    CurrentDateTime.SetAndSubscribe(update);

    setButtonSymbols(WidgetsDateTimeEdit::NoButtons);

    CurrentDateTime.OnMinMaxChanged.Connect(this, [this]{
        setDateTimeRange(CurrentDateTime.GetMinValid(), CurrentDateTime.GetMaxValid());
    });

    connect(this, &WidgetsDateTimeEdit::dateTimeChanged, [this]{
        if(m_recursionBlock || CurrentDateTime.IsRealTime()) {
            return;
        }
        guards::LambdaGuard guard([this]{ m_recursionBlock = false; }, [this]{ m_recursionBlock = true; });
        CurrentDateTime = dateTime();
    });

    WidgetsAttachment::Attach(this, [this](QObject*, QEvent* event){
        if(event->type() == QEvent::KeyPress) {
            auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Delete) {
                CurrentDateTime = QDateTime();
                return true;
            }
        }
        return false;
    });

    Locale.Subscribe([this]{
        setLocale(Locale);
        if(calendarWidget() != nullptr) {
            calendarWidget()->setLocale(Locale);
        }
    });

    DisplayFormat.Subscribe([this]{
        setDisplayFormat(DisplayFormat);
    });
}

WidgetsDateEdit::WidgetsDateEdit(QWidget* parent)
    : Super(QDate(2000,1,1), QVariant::Date, parent)
{
    CurrentDateTime.ConnectBoth(CurrentDate, [](const QDateTime& dateTime){
        if(!dateTime.isValid()) {
            return QDate();
        }
        return dateTime.date();
    }, [](const QDate& date){
        if(!date.isValid()) {
            return QDateTime();
        }
        return QDateTime(date, QTime(0,0));
    });

    CurrentDateTime.OnMinMaxChanged.Connect(this, [this]{
        CurrentDate.SetMinMax(CurrentDateTime.GetMin().date(), CurrentDateTime.GetMax().date());
    });

    CurrentDate.OnMinMaxChanged.Connect(this, [this]{
        CurrentDateTime.SetMinMax(CurrentDate.GetMin().isValid() ? QDateTime(CurrentDate.GetMin(), QTime(0,0)) : QDateTime(),
                                  CurrentDate.GetMax().isValid() ? QDateTime(CurrentDate.GetMax(), QTime(0,0)) : QDateTime());
    });
}
