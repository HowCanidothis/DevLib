#include "widgetsdatetimeedit.h"

#include <QKeyEvent>
#include <QCalendarWidget>

#include "WidgetsModule/Utils/styleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDateTimeEdit::WidgetsDateTimeEdit(QWidget* parent)
    : WidgetsDateTimeEdit(QDateTime(), QVariant::DateTime, parent)
{
}

WidgetsDateTimeEdit::WidgetsDateTimeEdit(const QVariant& date, QVariant::Type type, QWidget* parent)
    : Super(date, type, parent)
    , AutoResize(true)
{
    m_recursionBlock = false;
    setButtonSymbols(WidgetsDateTimeEdit::NoButtons);

    SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this](const QLocale& locale){
        setLocale(locale);
        if(calendarWidget() != nullptr) {
            calendarWidget()->setLocale(locale);
        }
        Resize();
    }).MakeSafe(m_connections);

    std::function<const LocalPropertyString&(const LanguageSettings&)> DisplayFormat;
    switch(type){
    case QVariant::Time: DisplayFormat = &LanguageSettings::TimeFormat; break;
    case QVariant::Date: DisplayFormat = &LanguageSettings::DateFormat; break;
    default: DisplayFormat = &LanguageSettings::DateTimeFormat; break;
    }
    DisplayFormat(SharedSettings::GetInstance().LanguageSettings).ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this](const QString& format){
        setDisplayFormat(format);
    }).MakeSafe(m_connections);

    CurrentDateTime.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this](const QDateTime& dt){
        if(m_recursionBlock) {
            return;
        }
        guards::BooleanGuard guard(&m_recursionBlock);
        QDateTime dateTime = TimeShift.IsValid ? dt.toOffsetFromUtc(TimeShift.Value) : dt;
        setDateTime(dateTime);
        if(CurrentDateTime.IsRealTime()) { // To update display value
            setDisplayFormat(displayFormat());
        }
        Resize();
    }, TimeShift);

    AutoResize.OnChanged += {this, [this]{
        Resize();
    }};

    CurrentDateTime.OnMinMaxChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::BooleanGuard guard(&m_recursionBlock);
        setDateTimeRange(CurrentDateTime.GetMinValid(), CurrentDateTime.GetMaxValid());
    });

    connect(this, &WidgetsDateTimeEdit::dateTimeChanged, [this](const QDateTime& dateTime){
        if(m_recursionBlock) {
            return;
        }
        guards::BooleanGuard guard(&m_recursionBlock);
        CurrentDateTime = TimeShift.IsValid ? dateTime.toOffsetFromUtc(TimeShift.Value) : dateTime;
    });

    WidgetWrapper(this).AddEventFilter([this](QObject*, QEvent* event){
        switch(event->type()){
            case QEvent::KeyPress: {
                auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
                switch(keyEvent->key()){
                case Qt::Key_Tab:
                case Qt::Key_Escape:
                case Qt::Key_Backtab:
                case Qt::Key_Backspace:
                case Qt::Key_Return:
                case Qt::Key_Enter:
                case Qt::Key_Period:
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_0:
                case Qt::Key_1:
                case Qt::Key_2:
                case Qt::Key_3:
                case Qt::Key_4:
                case Qt::Key_5:
                case Qt::Key_6:
                case Qt::Key_7:
                case Qt::Key_8:
                case Qt::Key_9: break;
                case Qt::Key_Delete: DefaultDateTimeDelegate();
                default: return true;
                }
            }
        }
        return false;
    });
}

QDateTime WidgetsDateTimeEdit::dateTimeFromText(const QString& text) const
{
    return !CurrentDateTime.IsRealTime() ? Super::dateTimeFromText(text) : QDateTime();
}

QString WidgetsDateTimeEdit::textFromDateTime(const QDateTime& dt) const
{
    return !CurrentDateTime.IsRealTime() ? Super::textFromDateTime(dt) : "";
}

QValidator::State WidgetsDateTimeEdit::validate(QString& input, int& pos) const
{
    return Super::validate(input, pos);
}

void WidgetsDateTimeEdit::Resize()
{
    if(AutoResize) {
        m_call.Call(CONNECTION_DEBUG_LOCATION, [this]{
            if(!AutoResize) {
                return;
            }
            QFontMetrics fm(font());
            auto t = text();
            int pixelsWide = fm.width(t);
            pixelsWide += contentsMargins().left() + contentsMargins().right() + 30;
            setMinimumWidth(pixelsWide);
        });
    } else {
        setMinimumWidth(0);
    }
}

WidgetsDateEdit::WidgetsDateEdit(QWidget* parent)
    : Super(QDate(2000,1,1), QVariant::Date, parent)
{
    CurrentDate.ConnectBoth(CONNECTION_DEBUG_LOCATION,CurrentDateTime, [](const QDate& date){
        if(!date.isValid()) {
            return QDateTime();
        }
        return QDateTime(date, QTime(0,0));
    }, [](const QDateTime& dateTime){
        if(!dateTime.isValid()) {
            return QDate();
        }
        return dateTime.date();
    });

    CurrentDateTime.OnMinMaxChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        CurrentDate.SetMinMax(CurrentDateTime.GetMin().date(), CurrentDateTime.GetMax().date());
    });

    CurrentDate.OnMinMaxChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        CurrentDateTime.SetMinMax(CurrentDate.GetMin().isValid() ? QDateTime(CurrentDate.GetMin(), QTime(0,0)) : QDateTime(),
                                  CurrentDate.GetMax().isValid() ? QDateTime(CurrentDate.GetMax(), QTime(0,0)) : QDateTime());
    });
}
