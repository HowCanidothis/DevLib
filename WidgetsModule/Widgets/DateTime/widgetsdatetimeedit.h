#ifndef WIDGETSDATETIMEEDIT_H
#define WIDGETSDATETIMEEDIT_H

#include <QDateTimeEdit>

#include <PropertiesModule/internal.hpp>

class WidgetsDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
    using Super = QDateTimeEdit;
public:
//    using Super::Super;
    WidgetsDateTimeEdit(QWidget* parent = nullptr);

    LocalPropertyDoubleOptional TimeShift;
    LocalPropertyDateTime CurrentDateTime;
    LocalPropertyString DisplayFormat;
    std::function<QDateTime ()> DefaultDateTimeDelegate = []{ return QDateTime(); };
    void Resize();

    QDateTime dateTimeFromText(const QString &text) const override;
    QString textFromDateTime(const QDateTime &dt) const override;
    QValidator::State validate(QString&, int&) const override;

protected:
    WidgetsDateTimeEdit(const QVariant& date, QVariant::Type type, QWidget* parent);

private:
    void init();
    virtual void updateLocale();

private:
    DispatcherConnectionsSafe m_connections;
    DelayedCallObject m_updateDisplayFormat;
    DelayedCallObject m_call;
    bool m_recursionBlock;
};

class WidgetsDateEdit : public WidgetsDateTimeEdit
{
    Q_OBJECT
    using Super = WidgetsDateTimeEdit;
public:
    WidgetsDateEdit(QWidget* parent = nullptr);

    LocalPropertyDate CurrentDate;

private:
    void updateLocale() override;
};

#endif // WIDGETSDATETIMEEDIT_H
