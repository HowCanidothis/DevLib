#ifndef WIDGETSIMPORTTABLEDIALOG_H
#define WIDGETSIMPORTTABLEDIALOG_H

#include <QDialog>

namespace Ui {
class WidgetsImportTableDialog;
}

class WidgetsImportTableDialog : public QDialog
{
    Q_OBJECT
    using Super = QDialog;

public:
    enum ImportResult {
        IR_Canceled,
        IR_Replaced,
        IR_Inserted,
    };

    explicit WidgetsImportTableDialog(QWidget *parent = nullptr);
    ~WidgetsImportTableDialog();

    class WidgetsImportView* GetView() const;

private slots:
    void on_BtnReplace_clicked();
    void on_BtnInsert_clicked();
    void on_BtnCancel_clicked();

private:
    Ui::WidgetsImportTableDialog *ui;
};

#endif // WIDGETSIMPORTTABLEDIALOG_H
