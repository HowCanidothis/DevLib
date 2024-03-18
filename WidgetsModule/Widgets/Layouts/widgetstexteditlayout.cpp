#include "widgetstexteditlayout.h"
#include "ui_widgetstexteditlayout.h"

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsTextEditLayout::WidgetsTextEditLayout(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsTextEditLayout)
{
    ui->setupUi(this);
    WidgetWrapper(ui->textEdit).ConnectFocus(ui->label);
    setFocusProxy(ui->textEdit);
}

WidgetsTextEditLayout::~WidgetsTextEditLayout()
{
    delete ui;
}

Qt::Orientation WidgetsTextEditLayout::orientation() const
{
    return ui->verticalLayout->direction() == QBoxLayout::TopToBottom ? Qt::Vertical : Qt::Horizontal;
}

void WidgetsTextEditLayout::setOrientation(Qt::Orientation orientation)
{
    if(this->orientation() != orientation) {
        if(orientation == Qt::Vertical) {
            ui->verticalLayout->setDirection(QBoxLayout::TopToBottom);
        } else {
            ui->verticalLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }
}

QLabel* WidgetsTextEditLayout::label() const
{
    return ui->label;
}

QString WidgetsTextEditLayout::title() const
{
    return ui->label->text();
}

QString WidgetsTextEditLayout::placeHolder() const
{
    return ui->textEdit->placeholderText();
}

QTextEdit* WidgetsTextEditLayout::textEdit() const
{
    return ui->textEdit;
}

void WidgetsTextEditLayout::setTitle(const QString& title)
{
    ui->label->setText(title);
}

void WidgetsTextEditLayout::setPlaceHolder(const QString& placeHolder)
{
    ui->textEdit->setPlaceholderText(placeHolder);
}

bool WidgetsTextEditLayout::readOnly() const
{
    return ui->textEdit->isReadOnly();
}

void WidgetsTextEditLayout::setReadOnly(bool readOnly)
{
    ui->textEdit->setReadOnly(readOnly);
}
