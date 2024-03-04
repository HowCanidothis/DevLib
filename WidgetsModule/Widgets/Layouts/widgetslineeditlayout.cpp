#include "widgetslineeditlayout.h"
#include "ui_widgetslineeditlayout.h"

WidgetsLineEditLayout::WidgetsLineEditLayout(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::WidgetsLineEditLayout)
{
    ui->setupUi(this);
}

WidgetsLineEditLayout::~WidgetsLineEditLayout()
{
    delete ui;
}

QLabel* WidgetsLineEditLayout::label()
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

QLineEdit* WidgetsLineEditLayout::lineEdit()
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
