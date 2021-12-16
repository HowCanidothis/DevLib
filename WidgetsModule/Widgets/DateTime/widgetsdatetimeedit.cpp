#include "widgetsdatetimeedit.h"

#include <QKeyEvent>
#include <QCalendarWidget>

#include "WidgetsModule/Utils/styleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDateTimeEdit::WidgetsDateTimeEdit(QWidget* parent)
    : Super(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    init();
}

WidgetsDateTimeEdit::WidgetsDateTimeEdit(const QVariant& date, QVariant::Type type, QWidget* parent)
    : Super(date, type, parent)
{
    setFocusPolicy(Qt::StrongFocus);
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
        setDateTime(CurrentDateTime);
        setDisplayFormat(displayFormat());
    };
    CurrentDateTime.SetAndSubscribe(update);

    setButtonSymbols(WidgetsDateTimeEdit::NoButtons);

    CurrentDateTime.OnMinMaxChanged.Connect(this, [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::LambdaGuard guard([this]{ m_recursionBlock = false; }, [this]{ m_recursionBlock = true; });
        setDateTimeRange(CurrentDateTime.GetMinValid(), CurrentDateTime.GetMaxValid());
    });

    connect(this, &WidgetsDateTimeEdit::dateTimeChanged, [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::LambdaGuard guard([this]{ m_recursionBlock = false; }, [this]{ m_recursionBlock = true; });
        CurrentDateTime = dateTime();
    });

    WidgetsAttachment::Attach(this, [this](QObject*, QEvent* event){
        if(event->type() == QEvent::Show) {
            connectLocale();
        }

        if(event->type() == QEvent::KeyPress) {
            auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Delete) {
                CurrentDateTime = DefaultDateTimeDelegate();
            }
            switch(keyEvent->key()){
            case Qt::Key_Tab: qDebug() << "tabletEvent";
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
            case Qt::Key_9: return false;
            default: break;
            }
            return true;
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
        guards::LambdaGuard guard([this]{ m_recursionBlock = false; }, [this]{ m_recursionBlock = true; });
        setDisplayFormat(DisplayFormat);
    });
}

void WidgetsDateTimeEdit::connectLocale()
{
    Locale.OnChange.ConnectAndCall(this, [this]{
        const auto& locale = Locale.Native();
        if(locale.language() == QLocale::English){
            DisplayFormat = "MM/dd/yy hh:mm AP";
        } else {
            DisplayFormat = locale.dateTimeFormat(QLocale::FormatType::ShortFormat);
        }
    });
}

WidgetsDateEdit::WidgetsDateEdit(QWidget* parent)
    : Super(QDate(2000,1,1), QVariant::Date, parent)
{
    CurrentDate.ConnectBoth(CurrentDateTime, [](const QDate& date){
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

    CurrentDateTime.OnMinMaxChanged.Connect(this, [this]{
        CurrentDate.SetMinMax(CurrentDateTime.GetMin().date(), CurrentDateTime.GetMax().date());
    });

    CurrentDate.OnMinMaxChanged.Connect(this, [this]{
        CurrentDateTime.SetMinMax(CurrentDate.GetMin().isValid() ? QDateTime(CurrentDate.GetMin(), QTime(0,0)) : QDateTime(),
                                  CurrentDate.GetMax().isValid() ? QDateTime(CurrentDate.GetMax(), QTime(0,0)) : QDateTime());
    });
}

void WidgetsDateEdit::connectLocale()
{
    Locale.OnChange.ConnectAndCall(this, [this]{
        const auto& locale = Locale.Native();
        if(locale.language() == QLocale::English){
            DisplayFormat = "MM/dd/yyyy";
        } else {
            DisplayFormat = locale.dateFormat(QLocale::FormatType::ShortFormat);
        }
    });
}

