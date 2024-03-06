#include "widgetslineeditlayout.h"
#include "ui_widgetslineeditlayout.h"

WidgetsLineEditLayout::WidgetsLineEditLayout(QWidget* parent)
    : Super(parent)
    , ui(new Ui::WidgetsLineEditLayout)
{
    ui->setupUi(this);
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
