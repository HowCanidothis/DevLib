#include "widgetscheckboxlayout.h"
#include "ui_widgetscheckboxlayout.h"

WidgetsCheckBoxLayout::WidgetsCheckBoxLayout(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::WidgetsCheckBoxLayout)
{
    ui->setupUi(this);
}

WidgetsCheckBoxLayout::~WidgetsCheckBoxLayout()
{
    delete ui;
}
