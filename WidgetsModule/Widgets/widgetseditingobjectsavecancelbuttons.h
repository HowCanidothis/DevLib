#ifndef WIDGETSEDITINGOBJECTSAVECANCELBUTTONS_H
#define WIDGETSEDITINGOBJECTSAVECANCELBUTTONS_H

#include <QFrame>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsEditingObjectSaveCancelButtons;
}

class WidgetsEditingObjectSaveCancelButtons : public QFrame
{
    Q_OBJECT

public:
    explicit WidgetsEditingObjectSaveCancelButtons(QWidget *parent = nullptr);
    ~WidgetsEditingObjectSaveCancelButtons();

    LocalPropertyPtr<class EditingObject> Object;

    Dispatcher OnAboutToBeHidden;

private slots:
    void on_BtnSave_clicked();
    void on_BtnCancel_clicked();
    void on_BtnDiff_clicked();

private:
    void hideEvent(QHideEvent* event) override;

private:
    Ui::WidgetsEditingObjectSaveCancelButtons *ui;
};

#endif // WIDGETSEDITINGOBJECTSAVECANCELBUTTONS_H
