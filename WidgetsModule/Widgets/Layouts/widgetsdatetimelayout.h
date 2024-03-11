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
    Q_PROPERTY(bool isDateTime READ isDateTime WRITE setIsDateTime)
    using Super = QFrame;
public:
    explicit WidgetsDateTimeLayout(QWidget *parent = nullptr);
    ~WidgetsDateTimeLayout();

    QLabel* label() const;
    WidgetsDateTimeWidget* popUp() const;
    WidgetsDateTimeEdit* dateTime() const;

    QString title() const;
    bool isDateTime() const;

    LocalPropertyBool ForceDisabled;
    LocalPropertyDoubleOptional TimeShift;

    Dispatcher OnCloseEditor;
    Dispatcher OnDataCommit;

public slots:
    void setTitle(const QString& title);
    void setIsDateTime(const bool& dateTime);

private:
    Ui::WidgetsDateTimeLayout *ui;
    WidgetsDateTimeWidget* m_editor;
    DispatcherConnectionsSafe m_connections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSDATETIMELAYOUT_H
