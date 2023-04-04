#include "widgetseditingobjectsavecancelbuttons.h"
#include "ui_widgetseditingobjectsavecancelbuttons.h"

#include <WidgetsModule/internal.hpp>

WidgetsEditingObjectSaveCancelButtons::WidgetsEditingObjectSaveCancelButtons(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsEditingObjectSaveCancelButtons)
{
    ui->setupUi(this);
    setProperty("Footer", true);
    setFocusPolicy(Qt::NoFocus);

    WidgetPushButtonWrapper(ui->BtnSave).SetControl(ButtonRole::Save);
    WidgetPushButtonWrapper(ui->BtnCancel).SetControl(ButtonRole::Reset);

    auto connections = DispatcherConnectionsSafeCreate();
    Object.Connect(CONNECTION_DEBUG_LOCATION, [this, connections](const EditingObject* o){
        connections->clear();
        ui->BtnSave->setVisible(o != nullptr);
        ui->BtnCancel->setVisible(o != nullptr);
        if(o == nullptr){
            return;
        }

        FocusManager::GetInstance().FocusedWidget().ConnectAndCall(CONNECTION_DEBUG_LOCATION, [this, o](const QWidget* widget){
            setVisible(o->IsDirty() && WidgetWrapper(this).HasParent(widget));
        }, const_cast<EditingObject*>(o)->OnDirtyChanged()).MakeSafe(*connections);

        WidgetPushButtonWrapper(ui->BtnSave).WidgetEnablity().ConnectFrom(CONNECTION_DEBUG_LOCATION, [](bool e){
            return !e;
        }, o->HasErrors.HasErrors).MakeSafe(*connections);
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
