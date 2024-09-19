#include "widgetsswitchlayout.h"
#include "ui_widgetsswitchlayout.h"

#include <QMouseEvent>
#include <QPushButton>

#include <WidgetsModule/internal.hpp>

WidgetsSwitchLayout::WidgetsSwitchLayout(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::WidgetsSwitchLayout),
    OnText(tr("On")),
    OffText(tr("Off"))
{
    ui->setupUi(this);

#ifdef QT_PLUGIN
    ui->pushButton->setObjectName("__qt__passive_button");
    ui->pushButton_2->setObjectName("__qt__passive_button2");
#endif

    m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(&IsOn, adapters::toVector(ui->pushButton,ui->pushButton_2));
    WidgetAbstractButtonWrapper(ui->pushButton).WidgetText()->ConnectFrom(CDL, OffText);
    WidgetAbstractButtonWrapper(ui->pushButton_2).WidgetText()->ConnectFrom(CDL, OnText);
    setFocusProxy(ui->pushButton);
}

QAbstractButton* WidgetsSwitchLayout::offButton() const
{
    return ui->pushButton;
}

QAbstractButton* WidgetsSwitchLayout::onButton() const
{
    return ui->pushButton_2;
}

WidgetsSwitchLayout::~WidgetsSwitchLayout()
{
}

void WidgetsSwitchLayout::setOn(bool on)
{
    IsOn = on;
}

bool WidgetsSwitchLayout::readOnly() const
{
    return !ui->pushButton->isEnabled();
}

void WidgetsSwitchLayout::setReadOnly(bool readOnly)
{
    ui->pushButton->setEnabled(!readOnly);
    ui->pushButton_2->setEnabled(!readOnly);
}
