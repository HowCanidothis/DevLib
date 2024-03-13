#include "widgetstexteditlayout.h"
#include "ui_widgetstexteditlayout.h"

WidgetsTextEditLayout::WidgetsTextEditLayout(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsTextEditLayout)
{
    ui->setupUi(this);
}

WidgetsTextEditLayout::~WidgetsTextEditLayout()
{
    delete ui;
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
