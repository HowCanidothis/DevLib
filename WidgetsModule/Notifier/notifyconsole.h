#ifndef NOTIFYCONSOLE_H
#define NOTIFYCONSOLE_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

#include "notifydeclarations.h"

namespace Ui {
class NotifyConsole;
}

class NotifyConsole : public QWidget
{
    Q_OBJECT

public:
    explicit NotifyConsole(QWidget *parent = nullptr);
    ~NotifyConsole();

    void SetCloseIcon(const class IconsSvgIcon& closeIcon);
    void SetCleanIcon(const IconsSvgIcon& cleanIcon);
    void SetWarningIcon(const IconsSvgIcon& cleanIcon);
    void AttachErrorsContainer(LocalPropertyErrorsContainer* container, const std::function<void (const Name&)>& handler);
    void DetachErrorsContainer(LocalPropertyErrorsContainer* container);

    NotifyConsoleDataWrapperPtr Data;
    LocalPropertyBool IsOpened;
    LocalPropertyBool IsShowWarnings;

private slots:
    void on_BtnCloseConsole_clicked();
    void on_BtnClear_clicked();

private:
    void erasePermanentErrors();

private:
    Ui::NotifyConsole *ui;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
    QHash<LocalPropertyErrorsContainer*, DispatcherConnectionsSafe> m_permanentErrors;
    DelayedCallObject m_erasePermanentErrors;
    QHash<LocalPropertyErrorsContainer*, QSet<Name>> m_permanentErrorsToErase;
    LocalPropertyColor m_showWarningsNormalColor;
    LocalPropertyColor m_showWarningsSelectedColor;
};

#endif // NOTIFYCONSOLE_H
