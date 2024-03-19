#ifndef WIDGETSDATETIMELAYOUT_H
#define WIDGETSDATETIMELAYOUT_H

#include <QFrame>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsDateTimeLayout;
}

class QLabel;
class WidgetsDateTimeEdit;
class WidgetsDateTimeWidget;
class WidgetsDateTimeLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(bool isDateTime READ isDateTime WRITE setIsDateTime)
    using Super = QFrame;
public:
    explicit WidgetsDateTimeLayout(QWidget *parent = nullptr);
    ~WidgetsDateTimeLayout();

    QLabel* label() const;
    class WidgetsDatetimePopupPicker* dateTime() const;

    QString title() const;
    bool isDateTime() const;

    bool readOnly() const;
    void setReadOnly(bool readOnly);

public slots:
    void setTitle(const QString& title);
    void setIsDateTime(const bool& dateTime);

private:
    Ui::WidgetsDateTimeLayout *ui;
};

#endif // WIDGETSDATETIMELAYOUT_H
