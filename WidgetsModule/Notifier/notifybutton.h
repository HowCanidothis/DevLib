#ifndef NOTIFYBUTTON_H
#define NOTIFYBUTTON_H

#include <QPushButton>

#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/Connectors/localpropertieswidgetconnector.h"

class NotifyButton : public QPushButton
{
    Q_OBJECT
    using Super = QPushButton;
    Q_PROPERTY(QSize offset MEMBER m_offset)

public:
    explicit NotifyButton(QWidget *parent = nullptr);
    ~NotifyButton();

    LocalPropertyInt NotificationsCount;

private:
    class QLabel* m_label;
    LocalPropertyString NotificationsCountString;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
    LocalProperty<QSize> m_offset;
};

#endif // NOTIFYBUTTON_H
