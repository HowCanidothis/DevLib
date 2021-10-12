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

    LocalPropertyDateTime CurrentDateTime;
    LocalPropertyLocale Locale;
    LocalPropertyString DisplayFormat;

    QDateTime dateTimeFromText(const QString &text) const override;
    QString textFromDateTime(const QDateTime &dt) const override;
    QValidator::State validate(QString&, int&) const override { return QValidator::Acceptable; }

protected:
    WidgetsDateTimeEdit(const QVariant& date, QVariant::Type type, QWidget* parent);

private:
    void init();

private:
    bool m_recursionBlock;
};

class WidgetsDateEdit : public WidgetsDateTimeEdit
{
    Q_OBJECT
    using Super = WidgetsDateTimeEdit;
public:
    WidgetsDateEdit(QWidget* parent = nullptr);

    LocalPropertyDate CurrentDate;
};

#endif // WIDGETSDATETIMEEDIT_H