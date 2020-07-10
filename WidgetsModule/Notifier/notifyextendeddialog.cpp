#include "notifyextendeddialog.h"
#include "ui_notifyextendeddialog.h"

NotifyExtendedDialog::NotifyExtendedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotifyExtendedDialog)
{
    ui->setupUi(this);
}

NotifyExtendedDialog::~NotifyExtendedDialog()
{
    delete ui;
}

void NotifyExtendedDialog::SetMessage(const QString& message)
{
    ui->messageLabel->setText(message);
}
