#include "widgetslineeditlayout.h"
#include "ui_widgetslineeditlayout.h"

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsLineEditLayout::WidgetsLineEditLayout(QWidget* parent)
    : Super(parent)
    , ui(new Ui::WidgetsLineEditLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->lineedit).ConnectFocus(ui->label);
    setFocusProxy(ui->lineedit);
}

WidgetsLineEditLayout::~WidgetsLineEditLayout()
{
    delete ui;
}

Qt::Orientation WidgetsLineEditLayout::orientation() const
{
    return ui->verticalLayout->direction() == QBoxLayout::TopToBottom ? Qt::Vertical : Qt::Horizontal;
}

void WidgetsLineEditLayout::setOrientation(Qt::Orientation orientation)
{
    if(this->orientation() != orientation) {
        if(orientation == Qt::Vertical) {
            ui->verticalLayout->setDirection(QBoxLayout::TopToBottom);
        } else {
            ui->verticalLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
}

QLabel* WidgetsLineEditLayout::label() const
{
    return ui->label;
}

QString WidgetsLineEditLayout::title() const
{
    return ui->label->text();
}

QString WidgetsLineEditLayout::placeHolder() const
{
    return ui->lineedit->placeholderText();
}

QLineEdit* WidgetsLineEditLayout::lineEdit() const
{
    return ui->lineedit;
}

void WidgetsLineEditLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsLineEditLayout::setPlaceHolder(const QString& placeHolder)
{
    ui->lineedit->setPlaceholderText(placeHolder);
}

bool WidgetsLineEditLayout::enableClearButton() const
{
    return ui->lineedit->isClearButtonEnabled();
}

void WidgetsLineEditLayout::setEnableClearButton(bool enabled)
{
    ui->lineedit->setClearButtonEnabled(enabled);
}

bool WidgetsLineEditLayout::readOnly() const
{
    return ui->lineedit->isReadOnly();
}

void WidgetsLineEditLayout::setReadOnly(bool readOnly)
{
    ui->lineedit->setReadOnly(readOnly);
}

WidgetsLineEditLayout::EchoMode WidgetsLineEditLayout::echoMode() const
{
    return ui->lineedit->echoMode();
}

void WidgetsLineEditLayout::setEchoMode(EchoMode mode){
    ui->lineedit->setEchoMode(mode);
}
