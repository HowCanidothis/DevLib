#ifndef WIDGETSDATETIMEEDIT_H
#define WIDGETSDATETIMEEDIT_H

#include <QDateTimeEdit>

#include <PropertiesModule/internal.hpp>

class WidgetsDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    using Super = QDateTimeEdit;
public:
    WidgetsDateTimeEdit(QWidget* parent = nullptr);

    LocalPropertyDoubleOptional TimeShift;
    LocalPropertyDateTime CurrentDateTime;
    std::function<QDateTime ()> DefaultDateTimeDelegate = []{ return QDateTime(); };
    void Resize();

    LocalPropertyDate& GetOrCreateDateProperty();

    LocalPropertyBool AutoResize;
    LocalPropertyInt Mode;

    QDateTime dateTimeFromText(const QString &text) const override;
    QString textFromDateTime(const QDateTime &dt) const override;
    QValidator::State validate(QString&, int&) const override;

protected:
    WidgetsDateTimeEdit(const QVariant& date, QVariant::Type type, QWidget* parent);

private:
    DispatcherConnectionsSafe m_connections;
    DelayedCallObject m_call;
    bool m_recursionBlock;
    ScopedPointer<LocalPropertyDate> m_dateProperty;
};

#endif // WIDGETSDATETIMEEDIT_H
