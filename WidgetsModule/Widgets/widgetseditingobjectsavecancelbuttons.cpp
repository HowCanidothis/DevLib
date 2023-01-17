#include "widgetseditingobjectsavecancelbuttons.h"
#include "ui_widgetseditingobjectsavecancelbuttons.h"

#include <WidgetsModule/internal.hpp>

WidgetsEditingObjectSaveCancelButtons::WidgetsEditingObjectSaveCancelButtons(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsEditingObjectSaveCancelButtons)
    , m_editingObject(nullptr)
{
    ui->setupUi(this);
    setProperty("Footer", true);
    setFocusPolicy(Qt::NoFocus);

    WidgetPushButtonWrapper(ui->BtnSave).SetControl(ButtonRole::Save);
    WidgetPushButtonWrapper(ui->BtnCancel).SetControl(ButtonRole::Reset);

#ifdef BUILD_MASTER
	ui->BtnDiff->setVisible(false);
#endif
}

void WidgetsEditingObjectSaveCancelButtons::hideEvent(QHideEvent*)
{
    OnAboutToBeHidden();
}

void WidgetsEditingObjectSaveCancelButtons::SetEditingObject(EditingObject* object)
{
    Q_ASSERT(m_editingObject == nullptr);
    m_editingObject = object;
    auto updateVisibility = [this]{
        auto* widget = FocusManager::GetInstance().FocusedWidget().Native();
        setVisible(m_editingObject->IsDirty() && WidgetWrapper(this).HasParent(widget));
    };
    auto updateEnablity = [this]{
        ui->BtnSave->setEnabled(!m_editingObject->HasErrors.HasErrors);
    };
    FocusManager::GetInstance().FocusedWidget().OnChanged.Connect(CONNECTION_DEBUG_LOCATION, updateVisibility).MakeSafe(m_connections);
    m_editingObject->OnDirtyChanged().Connect(CONNECTION_DEBUG_LOCATION, updateVisibility).MakeSafe(m_connections);
    m_editingObject->HasErrors.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, updateEnablity).MakeSafe(m_connections);
    updateVisibility();
    updateEnablity();
}

WidgetsEditingObjectSaveCancelButtons::~WidgetsEditingObjectSaveCancelButtons()
{
    delete ui;
}

void WidgetsEditingObjectSaveCancelButtons::on_BtnSave_clicked()
{
    m_editingObject->Save();
}

void WidgetsEditingObjectSaveCancelButtons::on_BtnCancel_clicked()
{
    m_editingObject->Discard();
}

void WidgetsEditingObjectSaveCancelButtons::on_BtnDiff_clicked()
{
    m_editingObject->DiffRequest();
}
