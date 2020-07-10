#ifndef NOTIFYEXTENDEDDIALOG_H
#define NOTIFYEXTENDEDDIALOG_H

#include <QDialog>

namespace Ui {
class NotifyExtendedDialog;
}

class NotifyExtendedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotifyExtendedDialog(QWidget *parent = 0);
    ~NotifyExtendedDialog();

    void SetMessage(const QString& message);

private:
    Ui::NotifyExtendedDialog *ui;
};

#endif // NOTIFYEXTENDEDDIALOG_H
