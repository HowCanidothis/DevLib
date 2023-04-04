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

QValidator::State WidgetsDateTimeEdit::validate(QString& input, int& pos) const
{
    return Super::validate(input, pos);
}

void WidgetsDateTimeEdit::Resize()
{
    m_call.Call(CONNECTION_DEBUG_LOCATION, [this]{
        QFontMetrics fm(font());
        auto t = text();
        int pixelsWide = fm.width(t);
        pixelsWide += contentsMargins().left() + contentsMargins().right() + 30;
        setMinimumWidth(pixelsWide);
    });
}

void WidgetsDateTimeEdit::init()
{
    m_recursionBlock = false;
    SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.Connect(CONNECTION_DEBUG_LOCATION, [this](const QLocale& locale){
        setLocale(locale);
        if(calendarWidget() != nullptr) {
            calendarWidget()->setLocale(locale);
        }
        Resize();
        m_updateDisplayFormat.Call(CONNECTION_DEBUG_LOCATION, [this]{
            updateLocale();
        });
    }).MakeSafe(m_connections);

    auto updateView = [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::BooleanGuard guard(&m_recursionBlock);
        QDateTime dateTime = TimeShift.IsValid ? CurrentDateTime.Native().toOffsetFromUtc(TimeShift.Value) : CurrentDateTime.Native();
        setDateTime(dateTime);
        setDisplayFormat(displayFormat());
        Resize();
    };

    CurrentDateTime.SetAndSubscribe(updateView);
    TimeShift.IsValid.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, updateView);
    TimeShift.Value.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, updateView);

    setButtonSymbols(WidgetsDateTimeEdit::NoButtons);

    CurrentDateTime.OnMinMaxChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::BooleanGuard guard(&m_recursionBlock);
        setDateTimeRange(CurrentDateTime.GetMinValid(), CurrentDateTime.GetMaxValid());
    });

    connect(this, &WidgetsDateTimeEdit::dateTimeChanged, [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::BooleanGuard guard(&m_recursionBlock);
        if(TimeShift.IsValid) {
            CurrentDateTime = QDateTime(dateTime().date(), dateTime().time(), Qt::OffsetFromUTC, TimeShift.Value);
            return;
        }
        CurrentDateTime = QDateTime(dateTime().date(), dateTime().time());
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

    DisplayFormat.Subscribe([this]{
        guards::BooleanGuard guard(&m_recursionBlock);
        setDisplayFormat(DisplayFormat);
        Resize();
    });
}

void WidgetsDateTimeEdit::updateLocale()
{
    const auto& locale = SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.Native();
    if(locale.language() == QLocale::English){
        DisplayFormat = "MM/dd/yy hh:mm AP";
    } else {
        DisplayFormat = locale.dateTimeFormat(QLocale::FormatType::ShortFormat);
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

void WidgetsDateEdit::updateLocale()
{
    const auto& locale = SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.Native();
    if(locale.language() == QLocale::English){
        DisplayFormat = "MM/dd/yyyy";
    } else {
        DisplayFormat = locale.dateFormat(QLocale::FormatType::ShortFormat);
    }
}

