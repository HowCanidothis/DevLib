#ifndef WIDGETSEDITINGOBJECTSAVECANCELBUTTONS_H
#define WIDGETSEDITINGOBJECTSAVECANCELBUTTONS_H

#include <QFrame>

namespace Ui {
class WidgetsEditingObjectSaveCancelButtons;
}

class WidgetsEditingObjectSaveCancelButtons : public QFrame
{
    Q_OBJECT

public:
    explicit WidgetsEditingObjectSaveCancelButtons(QWidget *parent = nullptr);
    ~WidgetsEditingObjectSaveCancelButtons();

    void SetEditingObject(class EditingObject* object);

    Dispatcher OnAboutToBeHidden;

private slots:
    void on_BtnSave_clicked();
    void on_BtnCancel_clicked();
    void on_BtnDiff_clicked();

private:
    void hideEvent(QHideEvent* event) override;

private:
    Ui::WidgetsEditingObjectSaveCancelButtons *ui;
    EditingObject* m_editingObject;
    DispatcherConnectionsSafe m_connections;
};

#endif // WIDGETSEDITINGOBJECTSAVECANCELBUTTONS_H
