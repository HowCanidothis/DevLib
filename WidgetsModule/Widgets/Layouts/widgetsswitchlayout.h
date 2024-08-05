#ifndef WIDGETSSWITCHLAYOUT_H
#define WIDGETSSWITCHLAYOUT_H

#include <QFrame>

#include <WidgetsModule/internal.hpp>

namespace Ui {
class WidgetsSwitchLayout;
}

class WidgetsSwitchLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(bool on READ on WRITE setOn)
    Q_PROPERTY(QString onText MEMBER OnText)
    Q_PROPERTY(QString offText MEMBER OffText)
public:
    explicit WidgetsSwitchLayout(QWidget *parent = nullptr);
    ~WidgetsSwitchLayout();

    LocalPropertyInt IsOn;
    LocalPropertyString OnText;
    LocalPropertyString OffText;

    void setOn(bool on);
    bool on() const { return IsOn; }

    bool readOnly() const;
    void setReadOnly(bool readOnly);

    // QWidget interface
protected:
    Ui::WidgetsSwitchLayout* ui;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSSWITCHLAYOUT_H
