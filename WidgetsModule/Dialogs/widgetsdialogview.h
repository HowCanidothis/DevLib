#ifndef WidgetsDialogView_H
#define WidgetsDialogView_H

#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>

namespace Ui {
class WidgetsDialogView;
}

class WidgetsDialogView : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit WidgetsDialogView(QWidget *parent = nullptr);
    ~WidgetsDialogView();

    void SetContent(QWidget* view);
    void AddAction(QDialogButtonBox::ButtonRole role, const QString& text);

private:
    Ui::WidgetsDialogView *ui;
    qint32 m_buttonsCount;
};

#endif // WidgetsDialogView_H
