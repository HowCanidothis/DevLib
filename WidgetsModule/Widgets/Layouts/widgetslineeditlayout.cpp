#include "widgetslineeditlayout.h"
#include "ui_widgetslineeditlayout.h"

#include <WidgetsModule/internal.hpp>

WidgetsLineEditLayout::WidgetsLineEditLayout(QWidget* parent)
    : Super(parent)
    , ui(new Ui::WidgetsLineEditLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->lineEdit).ConnectFocus(ui->label);
}

WidgetsLineEditLayout::~WidgetsLineEditLayout()
{
    delete ui;
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
    return ui->lineEdit->placeholderText();
}

QLineEdit* WidgetsLineEditLayout::lineEdit() const
{
    return ui->lineEdit;
}

void WidgetsLineEditLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsLineEditLayout::setPlaceHolder(const QString& placeHolder)
{
    ui->lineEdit->setPlaceholderText(placeHolder);
}

bool WidgetsLineEditLayout::enableClearButton() const
{
    return ui->lineEdit->isClearButtonEnabled();
}

void WidgetsLineEditLayout::setEnableClearButton(bool enabled)
{
    ui->lineEdit->setClearButtonEnabled(enabled);
}

bool WidgetsLineEditLayout::readOnly() const
{
    return ui->lineEdit->isReadOnly();
}

void WidgetsLineEditLayout::setReadOnly(bool readOnly)
{
    ui->lineEdit->setReadOnly(readOnly);
}

WidgetsLineEditLayout::EchoMode WidgetsLineEditLayout::echoMode() const
{
    return ui->lineEdit->echoMode();
}

void WidgetsLineEditLayout::setEchoMode(EchoMode mode){
    ui->lineEdit->setEchoMode(mode);
}
