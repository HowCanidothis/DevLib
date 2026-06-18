#include "widgetseditingobjectsavecancelbuttons.h"
#include "ui_widgetseditingobjectsavecancelbuttons.h"

#include <WidgetsModule/internal.hpp>

WidgetsEditingObjectSaveCancelButtons::WidgetsEditingObjectSaveCancelButtons(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsEditingObjectSaveCancelButtons)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::NoFocus);

    WidgetAbstractButtonWrapper(ui->BtnSave).SetControl(ButtonRole::IconWithText);
    WidgetAbstractButtonWrapper(ui->BtnCancel).SetControl(ButtonRole::IconWithText);

    auto connections = DispatcherConnectionsSafeCreate();
    Object.Connect(CDL, [this, connections](const EditingObject* o){
        connections->clear();
        ui->BtnSave->setVisible(o != nullptr);
        ui->BtnCancel->setVisible(o != nullptr);
        if(o == nullptr){
            return;
        }

        FocusManager::GetInstance().FocusedWidget().ConnectAndCall(CDL, [this, o](const QWidget* widget){
            setVisible(o->IsDirty() && WidgetWrapper(this).HasParent(widget));
        }, const_cast<EditingObject*>(o)->OnDirtyChanged()).MakeSafe(*connections);

        WidgetAbstractButtonWrapper(ui->BtnSave).WidgetEnablity().ConnectFromDispatchers(CDL, [o]{
            return o->ErrorsModel.IsEmpty();
        }, o->ErrorsModel.OnChanged).MakeSafe(*connections);
    });
#ifdef BUILD_MASTER
	ui->BtnDiff->setVisible(false);
#endif
}

void WidgetsEditingObjectSaveCancelButtons::hideEvent(QHideEvent*)
{
    OnAboutToBeHidden();
}

WidgetsEditingObjectSaveCancelButtons::~WidgetsEditingObjectSaveCancelButtons()
{
    delete ui;
}

void WidgetsEditingObjectSaveCancelButtons::on_BtnSave_clicked()
{
    if(Object.Native() != nullptr) Object->Save();
}

void WidgetsEditingObjectSaveCancelButtons::on_BtnCancel_clicked()
{
    if(Object.Native() != nullptr) Object->Discard();
}

void WidgetsEditingObjectSaveCancelButtons::on_BtnDiff_clicked()
{
    if(Object.Native() != nullptr) Object->DiffRequest();
}
