#ifndef WIDGETSDATETIMEPOPUPPICKER_H
#define WIDGETSDATETIMEPOPUPPICKER_H

#include <QFrame>
#include <QDateTimeEdit>
#include <PropertiesModule/internal.hpp>
#include "WidgetsModule/widgetsdeclarations.h"

namespace Ui {
class WidgetsDatetimePopupPicker;
}

class WidgetsDatetimePopupPicker : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qint32 dateMode READ dateMode WRITE setDateMode)
    using Super = QFrame;
public:
    explicit WidgetsDatetimePopupPicker(QWidget *parent = nullptr);
    ~WidgetsDatetimePopupPicker() override;

    class WidgetsDateTimeWidget* GetDateTimeWidget() const { return m_editor; }
    class WidgetsDateTimeEdit* GetLineEdit() const;
    class QPushButton* GetCalendarButton() const;

    qint32 dateMode() const { return Mode; }
    void setDateMode(qint32 mode) { Mode = mode; }

    LocalPropertyDoubleOptional TimeShift;

    LocalPropertySequentialEnum<DateTimeDisplayFormatEnum> Mode;
    Dispatcher OnCloseEditor;
    Dispatcher OnDataCommit;

    LocalPropertyBool ForceDisabled;

private:
    Ui::WidgetsDatetimePopupPicker *ui;
    WidgetsDateTimeWidget* m_editor;
    DispatcherConnectionsSafe m_connections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSDATETIMEPOPUPPICKER_H
