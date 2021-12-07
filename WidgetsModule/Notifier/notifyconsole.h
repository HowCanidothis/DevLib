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
    enum ElementVisibility
    {
        ElementVisibility_Close = 0x1,
        ElementVisibility_Clear = 0x2,
        ElementVisibility_ShowWarnings = 0x4,
        ElementVisibility_Filter = 0x8,
        ElementVisibility_ShowInfos = 0x10,
        ElementVisibility_ShowErrors = 0x20
    };
    DECL_FLAGS(ElementVisibilityFlags, ElementVisibility);

    explicit NotifyConsole(QWidget *parent = nullptr);
    ~NotifyConsole();

    void SetVisibility(ElementVisibilityFlags visibility);
    void SetCloseIcon(const class IconsSvgIcon& closeIcon);
    void SetErrorIcon(const IconsSvgIcon& errorIcon);
    void SetCleanIcon(const IconsSvgIcon& cleanIcon);
    void SetWarningIcon(const IconsSvgIcon& cleanIcon);
    void SetInfoIcon(const IconsSvgIcon& icon);

    void AddWarning(const QString& data, const FAction& handler);

    void AttachErrorsContainer(LocalPropertyErrorsContainer* container, const std::function<void (const Name&)>& handler);
    void DetachErrorsContainer(LocalPropertyErrorsContainer* container);

    NotifyConsoleDataWrapperPtr Data;
    LocalPropertyBool IsOpened;
    LocalPropertyBool IsShowWarnings;
    LocalPropertyBool IsShowInfos;
    LocalPropertyBool IsShowErrors;
    DelayedCallCommonDispatcher<qint32> OnShownMessagesCountChanged;

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
    DelayedCallDispatcher m_updateErrors;
};

#endif // NOTIFYCONSOLE_H
