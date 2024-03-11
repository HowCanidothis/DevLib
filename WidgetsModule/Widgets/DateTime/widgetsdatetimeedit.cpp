#include "widgetsdatetimeedit.h"

#include <QKeyEvent>
#include <QCalendarWidget>

#include "WidgetsModule/Utils/widgetstyleutils.h"
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

    if(SharedSettings::IsInitialized()) {
        SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this](const QLocale& locale){
            setLocale(locale);
            if(calendarWidget() != nullptr) {
                calendarWidget()->setLocale(locale);
            }
            Resize();
        }).MakeSafe(m_connections);

        if(type == QVariant::DateTime) {
            auto displayTextConnections = DispatcherConnectionsSafeCreate();
            Mode.ConnectAndCall(CDL, [this, displayTextConnections](int mode){
                displayTextConnections->clear();
                LocalPropertyString* displayFormat;
                switch(mode){
                case 1/*WidgetsDateTimeWidget::Date*/: displayFormat = &SharedSettings::GetInstance().LanguageSettings.DateFormat; break;
                case 2/*WidgetsDateTimeWidget::Time*/: displayFormat = &SharedSettings::GetInstance().LanguageSettings.TimeFormat; break;
                default: displayFormat = &SharedSettings::GetInstance().LanguageSettings.DateTimeFormat; break;
                }
                displayFormat->ConnectAndCall(CDL, [this](const QString& format){
                    setDisplayFormat(format);
                }).MakeSafe(*displayTextConnections);
            }).MakeSafe(m_connections);
        } else {
            SharedSettings::GetInstance().LanguageSettings.DateFormat.ConnectAndCall(CDL, [this](const QString& format){
                setDisplayFormat(format);
            }).MakeSafe(m_connections);
        }
    }

    CurrentDateTime.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this](const QDateTime& dt){
        QDateTime dateTime = TimeShift.IsValid ? dt.toOffsetFromUtc(TimeShift.Value) : dt;
        if(m_recursionBlock && this->dateTime() == dateTime) {
            return;
        }
        guards::BooleanGuard guard(&m_recursionBlock);

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
        CurrentDateTime = TimeShift.IsValid ? QDateTime(dateTime.date(), dateTime.time(), Qt::OffsetFromUTC, TimeShift.Value) : dateTime;
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
                case Qt::Key_Slash:
                case Qt::Key_V:
                case Qt::Key_C:
                case Qt::Key_9: break;
                case Qt::Key_Delete:
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
