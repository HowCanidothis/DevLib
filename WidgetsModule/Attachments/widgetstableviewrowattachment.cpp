#include "widgetstableviewrowattachment.h"

#include <QTableView>
#include <QAction>

#include "WidgetsModule/Managers/widgetsdialogsmanager.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Actions/widgetsglobaltableactionsscope.h"

void WidgetsTableViewRowAttachment::ConnectButton(const Latin1Name& action, const WidgetPushButtonWrapper& button, const FTranslationHandler& dialogText, const WidgetsDialogsManagerButtonStruct& confirmButton)
{
    button.SetOnClicked([this, action, dialogText, confirmButton]{
        if(dialogText != nullptr) {
            if(!WidgetsDialogsManager::GetInstance().ShowTempDialog(DescCustomDialogParams().SetTitle(dialogText)
                                                                    .AddButton(WidgetsDialogsManagerDefaultButtons::CancelButton())
                                                                    .AddButton(confirmButton))) {
                return;
            }
        }
        WidgetWrapper(m_target->viewport()).Click();
        if(SelectCurrentRow()) {
            WidgetsGlobalTableActionsScope::GetInstance().FindAction(action)->trigger();
        }
    });
}

WidgetsTableViewRowAttachment::WidgetsTableViewRowAttachment(const std::function<QWidget* (WidgetsTableViewRowAttachment*)>& widgetCreator)
    : CurrentRow(-1)
    , m_target(nullptr)
    , m_pane(widgetCreator(this))
{
    Q_ASSERT(m_pane->parent() == nullptr);
    m_pane->setObjectName("WidgetsTVRAWidget");
    StyleUtils::InstallSizeAdjuster(m_pane);
}

void WidgetsTableViewRowAttachment::Attach(QTableView* v)
{
    v->setMouseTracking(true);
    v->connect(v->viewport(), &QTableView::destroyed, [this,v]{
        if(m_pane->parent() == v->viewport()) {
            m_pane->setParent(nullptr);
        }
    });
    v->connect(v, &QTableView::entered, [this, v](const QModelIndex& index) {
        if(!index.isValid() || index.data(LastEditRowRole).toBool()) {
            m_pane->hide();
            CurrentRow = -1;
            m_pane->setParent(nullptr);
            m_target = nullptr;
            return;
        }
        m_pane->setParent(v->viewport());
        m_pane->show();
        QRect rect = v->visualRect(index);
        auto left = v->viewport()->width() - m_pane->width();
        m_pane->move(left, rect.top() + (rect.height() - m_pane->height()) / 2);
        CurrentRow = index.row();
        m_target = v;
    });
    v->connect(v, &QTableView::viewportEntered, [this] {
        m_pane->hide();
        CurrentRow = -1;
        m_pane->setParent(nullptr);
        m_target = nullptr;
    });
}

bool WidgetsTableViewRowAttachment::SelectCurrentRow()
{
    if(CurrentRow == -1) {
        return false;
    }
    m_target->selectRow(CurrentRow);
    return true;
}
