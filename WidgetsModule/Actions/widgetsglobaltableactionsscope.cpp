#include "widgetsglobaltableactionsscope.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QAction>
#include <QTableView>

#include <ActionsModule/internal.hpp>

#include "WidgetsModule/Attachments/widgetsactivetableattachment.h"
#include "WidgetsModule/Utils/widgethelpers.h"

struct WidgetsGlobalTableActionsScopeHandlerData
{
    FAction Action;
    LocalPropertyBool Visibility;
    LocalPropertyBool Enablity;
    TranslatedString Text;

    WidgetsGlobalTableActionsScopeHandlerData(const FAction& action)
        : Action(action)
        , Visibility(false)
        , Enablity(false)
    {

    }
};

WidgetsGlobalTableActionsScopeHandler& WidgetsGlobalTableActionsScopeHandler::SetVisible(bool visible)
{
    m_data->Visibility = visible;
    m_data->Enablity = visible;
    return *this;
}

WidgetsGlobalTableActionsScopeHandler::WidgetsGlobalTableActionsScopeHandler(const FAction& action)
    : m_data(::make_shared<WidgetsGlobalTableActionsScopeHandlerData>(action))
{
}

LocalPropertyBool& WidgetsGlobalTableActionsScopeHandler::Visibility() const { return m_data->Visibility; }
LocalPropertyBool& WidgetsGlobalTableActionsScopeHandler::Enablity() const { return m_data->Enablity; }
TranslatedString& WidgetsGlobalTableActionsScopeHandler::Text() const { return m_data->Text; }

WidgetsGlobalTableActionsScopeHandler& WidgetsGlobalTableActionsScopeHandler::SetAction(const FAction& action)
{
    m_data->Action = action;
    return *this;
}

static QVector<QStringList> clipboardData()
{
    QVector<QStringList> ret;

    QClipboard* clipboard = qApp->clipboard();
    auto rows = clipboard->text().split('\n');
    for(const auto& row : rows){
        ret.append(row.split('\t'));
    }
    return ret;
}

WidgetsGlobalTableActionsScope::WidgetsGlobalTableActionsScope()
    : Super("TableEdit")
    , Singletone<WidgetsGlobalTableActionsScope>(this)
{
    if(!WidgetsGlobalTableActionId::m_delayedRegistration.isEmpty()) {
        for(const auto& [id, mode] : WidgetsGlobalTableActionId::m_delayedRegistration) {
            registerAction(id, mode);
        }

        WidgetsGlobalTableActionId::m_delayedRegistration.clear();
    }

    auto updateActiveTable = [this]{
        auto& activeTable = WidgetsActiveTableViewAttachment::GetInstance()->ActiveTable;

        if(activeTable == nullptr) {
            m_currentHandlers = nullptr;
            return;
        }
        m_currentHandlers = WidgetWrapper(activeTable).Injected<WidgetsGlobalTableActionsScopeHandlers>("a_tableActionsHandlers");
    };

    auto updateHandlers = [this]{
        m_currentHandlersConnections.clear();

        auto offAction = [](QAction* action) {
            ActionWrapper(action).WidgetVisibility() = false;
            ActionWrapper(action).WidgetEnablity() = false;
        };

        if(m_currentHandlers == nullptr) {
            for(auto* action : GetActions()) {
                offAction(action);
            }
            return;
        }

        for(auto* action : GetActions()) {
            auto handler = m_currentHandlers->Handlers.find(action);
            if(handler == m_currentHandlers->Handlers.end()) {
                offAction(action);
                continue;
            }

            (*ActionWrapper(action).Injected<FConnector>("a_connector"))(*handler);
        }
    };

    m_currentHandlers.Subscribe(updateHandlers);
    WidgetsActiveTableViewAttachment::GetInstance()->ActiveTable.SetAndSubscribe(updateActiveTable);
}

QVector<std::pair<Latin1Name, WidgetsGlobalTableActionsScope::EnableIfMode>> WidgetsGlobalTableActionId::m_delayedRegistration;

FAction WidgetsGlobalTableActionsScope::CreateDefaultDeleteHandler(QTableView* table)
{
    return [table]{
        auto* model = table->model();
        auto indexs = WidgetTableViewWrapper(table).SelectedRowsSorted();
        if(indexs.isEmpty()){
            return ;
        }
        int startSeries = indexs.first();
        int counter = startSeries + 1;

        QVector<std::pair<qint32,qint32>> toRemove; // first - from, second - count
        for(const auto& index : adapters::range(indexs.begin() + 1, indexs.end())){
            if(counter != index){
                toRemove.append({startSeries, counter - startSeries});
                startSeries = index;
                counter = startSeries + 1;
            } else {
                ++counter;
            }
        }
        toRemove.append({startSeries, counter - startSeries});
        while(!toRemove.isEmpty()){
            const auto& info = toRemove.takeLast();
            model->removeRows(info.first, info.second);
        }
    };
}

FAction WidgetsGlobalTableActionsScope::CreateDefaultPasteHandler(QTableView* table)
{
    return [table]{
            auto setModelData = [](QTableView* tableView, const QModelIndex& currentIndex, const QVector<QStringList>& data){
                if(!currentIndex.isValid()) {
                    return;
                }
                auto* model = tableView->model();
                const auto* header = tableView->horizontalHeader();

                auto currentRow = currentIndex.row();
                QVector<std::pair<qint32,qint32>> toSelect;
                for(const auto& row : data) {
                    auto currentColumn = header->visualIndex(currentIndex.column());
                    for(const auto& value : row) {
                        auto li = header->logicalIndex(currentColumn);
                        if(header->isSectionHidden(li)){
                            currentColumn++;
                            continue;
                        }

                        auto index = model->index(currentRow, li);
                        if(index.flags().testFlag(Qt::ItemIsEditable)) {
                            QLocale currentLocale;
                            bool ok;
                            auto doubleValue = currentLocale.toDouble(value, &ok);
                            if(ok) {
                                model->setData(index, doubleValue);
                            } else {
                                model->setData(index, value);
                            }
                            toSelect.append(std::make_pair(currentRow, li));
                        }
                        currentColumn++;
                    }
                    currentRow++;
                }
                QItemSelection selection;
                for(const auto& item : toSelect) {
                    selection.append(QItemSelectionRange(model->index(item.first, item.second)));
                }
                tableView->selectionModel()->select(selection, QItemSelectionModel::Clear | QItemSelectionModel::Select);
            };
            setModelData(table, WidgetTableViewWrapper(table)->currentIndex(), clipboardData());
        };
}

QList<QString> WidgetsGlobalTableActionsScope::ClipboardRows()
{
    QList<QString> ret;

    QClipboard* clipboard = qApp->clipboard();
    auto rows = clipboard->text().split('\n');
    for(const auto& row : rows){
        ret.append(row);
    }
    return ret;
}

QAction* WidgetsGlobalTableActionsScope::registerAction(const Latin1Name& id, EnableIfMode mode)
{
    Q_ASSERT(FindAction(id) == nullptr);
    auto* action = createAction(id, [this, id]{
        auto* action = FindAction(id);
        if(m_currentHandlers == nullptr) {
            Q_ASSERT(false);
            return;
        }
        auto foundIt = m_currentHandlers->Handlers.find(action);
        if(foundIt == m_currentHandlers->Handlers.end()) {
            return;
        }
        foundIt.value().m_data->Action();
    });
    switch(mode)
    {
    case EIM_TableSelectionOnlyOne:
        ActionWrapper(action).Injected<FConnector>("a_connector", [this, action]() -> FConnector* {
            return new FConnector([this, action](const WidgetsGlobalTableActionsScopeHandler& handler){
                auto& hasSelection = WidgetsActiveTableViewAttachment::GetInstance()->SelectedRowsCount;
                ActionWrapper(action).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Visibility()).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetEnablity().ConnectFrom(CONNECTION_DEBUG_LOCATION, [](bool enabled, qint32 hasSelection){
                    return hasSelection == 1 && enabled;
                }, handler.Enablity(), hasSelection).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetText()->ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Text()).MakeSafe(m_currentHandlersConnections);
            });
        });
    case EIM_TableSelectionOnlyOneAndIsEditable:
        ActionWrapper(action).Injected<FConnector>("a_connector", [this, action]() -> FConnector* {
            return new FConnector([this, action](const WidgetsGlobalTableActionsScopeHandler& handler){
                auto& isReadOnly = m_currentHandlers->IsReadOnly;
                auto& selectedRowsCount = WidgetsActiveTableViewAttachment::GetInstance()->SelectedRowsCount;
                ActionWrapper(action).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Visibility()).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetEnablity().ConnectFrom(CONNECTION_DEBUG_LOCATION, [](bool enabled, qint32 selected, qint32 readOnly){
                    return !readOnly && enabled && selected == 1;
                }, handler.Enablity(), selectedRowsCount, isReadOnly).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetText()->ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Text()).MakeSafe(m_currentHandlersConnections);
            });
        });
    case EIM_TableHasSelectionAndIsEditable:
        ActionWrapper(action).Injected<FConnector>("a_connector", [this, action]() -> FConnector* {
            return new FConnector([this, action](const WidgetsGlobalTableActionsScopeHandler& handler){
                auto& isReadOnly = m_currentHandlers->IsReadOnly;
                auto& hasSelection = WidgetsActiveTableViewAttachment::GetInstance()->HasSelection;
                ActionWrapper(action).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Visibility()).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetEnablity().ConnectFrom(CONNECTION_DEBUG_LOCATION, [](bool enabled, bool hasSelection, bool readOnly){
                    return !readOnly && enabled && hasSelection;
                }, handler.Enablity(), hasSelection, isReadOnly).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetText()->ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Text()).MakeSafe(m_currentHandlersConnections);
            });
        });
    case EIM_TableIsEditable:
        ActionWrapper(action).Injected<FConnector>("a_connector", [this, action]() -> FConnector* {
            return new FConnector([this, action](const WidgetsGlobalTableActionsScopeHandler& handler){
                ActionWrapper(action).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Visibility()).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetEnablity().ConnectFrom(CONNECTION_DEBUG_LOCATION, [](bool enabled, bool readOnly){
                    return !readOnly && enabled;
                }, handler.Enablity(), m_currentHandlers->IsReadOnly).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetText()->ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Text()).MakeSafe(m_currentHandlersConnections);
            });
        });
    case EIM_TableHasSelection:
        ActionWrapper(action).Injected<FConnector>("a_connector", [this, action]() -> FConnector* {
            return new FConnector([this, action](const WidgetsGlobalTableActionsScopeHandler& handler){
                auto& hasSelection = WidgetsActiveTableViewAttachment::GetInstance()->HasSelection;
                ActionWrapper(action).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Visibility()).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetEnablity().ConnectFrom(CONNECTION_DEBUG_LOCATION, [handler](bool enabled, bool hasSelection){
                    return hasSelection && enabled;
                }, handler.Enablity(), hasSelection).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetText()->ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Text()).MakeSafe(m_currentHandlersConnections);
            });
        });
    default:
        ActionWrapper(action).Injected<FConnector>("a_connector", [this, action]() -> FConnector* {
            return new FConnector([this, action](const WidgetsGlobalTableActionsScopeHandler& handler){
                ActionWrapper(action).WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Visibility()).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetEnablity().ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Enablity()).MakeSafe(m_currentHandlersConnections);
                ActionWrapper(action).WidgetText()->ConnectFrom(CONNECTION_DEBUG_LOCATION, handler.Text()).MakeSafe(m_currentHandlersConnections);
            });
        });
        break;
    }

    return action;
}

WidgetsGlobalTableActionsScopeHandlersPtr WidgetsGlobalTableActionsScope::EditHandlers(QTableView* table)
{
    return WidgetWrapper(table).Injected<WidgetsGlobalTableActionsScopeHandlers>("a_tableActionsHandlers", []{
        return new WidgetsGlobalTableActionsScopeHandlers();
    });
}

WidgetsGlobalTableActionsScopeHandlersPtr WidgetsGlobalTableActionsScope::AddDefaultHandlers(QTableView* table, bool assert)
{
    auto result = WidgetWrapper(table).Injected<WidgetsGlobalTableActionsScopeHandlers>("a_tableActionsHandlers", []{
        return new WidgetsGlobalTableActionsScopeHandlers();
    });

    Q_ASSERT(!assert || result->Handlers.isEmpty());

    auto action = GetInstance().FindAction(GlobalActionCopyWithHeadersId);
    result->AddHandler([]{ return tr("Copy With Headers"); }, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C), action, [table]{
        WidgetTableViewWrapper(table).CopySelectedTableContentsToClipboard(true);
    });
#ifndef BUILD_MASTER
    action = GetInstance().FindAction(GlobalActionDebugJSONId);
    result->AddHandler([]{ return tr("Debug JSON"); }, QKeySequence(), action, [table]{
        WidgetTableViewWrapper(table).DebugJson();
    }).SetVisible(true);

    action = GetInstance().FindAction(GlobalActionDebugSelectId);
    result->AddHandler([]{ return tr("Debug Select Random Rows"); }, QKeySequence(), action, [table]{
        WidgetTableViewWrapper(table).DebugSelect();
    }).SetVisible(true);
#endif

    action = GetInstance().FindAction(GlobalActionCopyId);
    auto copyHandler = [table]{
        WidgetTableViewWrapper(table).CopySelectedTableContentsToClipboard();
    };

    result->AddHandler([]{ return tr("Copy"); }, QKeySequence(Qt::CTRL + Qt::Key_C), action, copyHandler);

    action = GetInstance().FindAction(GlobalActionPasteId);
    result->AddHandler([]{ return tr("Paste"); }, QKeySequence(Qt::CTRL + Qt::Key_V), action, CreateDefaultPasteHandler(table));


    action = GetInstance().FindAction(GlobalActionDeleteId);
    auto deleteHandler = CreateDefaultDeleteHandler(table);

    result->AddHandler([]{ return tr("Delete Row(s)"); }, QKeySequence(Qt::SHIFT + Qt::Key_Delete), action, deleteHandler);

    action = GetInstance().FindAction(GlobalActionCutId);
    result->AddHandler([]{ return tr("Cut"); }, QKeySequence(Qt::CTRL + Qt::Key_X), action, [copyHandler, deleteHandler]{
        copyHandler();
        deleteHandler();
    });

    action = GetInstance().FindAction(GlobalActionInsertId);
    result->AddHandler([]{ return tr("Insert"); }, QKeySequence(Qt::CTRL + Qt::Key_Insert), action, [table]{
        auto* model = table->model();
        auto selectedRows = WidgetTableViewWrapper(table).SelectedRowsSorted();
        int rowIndex = model->rowCount();

        if(!selectedRows.isEmpty()){
            rowIndex = selectedRows.last();
        }

        model->insertRow(rowIndex);
    });

    return result;
}

WidgetsGlobalTableActionsScopeHandlers& WidgetsGlobalTableActionsScopeHandlers::ShowAll()
{
    for(const auto& handler : Handlers) {
        handler.Visibility() = true;
        handler.Enablity() = true;
    }
    return *this;
}

WidgetsGlobalTableActionsScopeHandlers& WidgetsGlobalTableActionsScopeHandlers::SetActionsVisible(const QVector<Latin1Name>& ids, bool visible)
{
    for(const auto& id : ids) {
        auto handler = FindHandler(id);
        handler.SetVisible(visible);
    }
    return *this;
}

WidgetsGlobalTableActionsScopeHandler WidgetsGlobalTableActionsScopeHandlers::AddHandler(const FTranslationHandler& handler, const QKeySequence& sequence, QAction* action, const FAction& actionHandler)
{
    auto ret = *Handlers.insert(action, actionHandler);
    action->setShortcut(sequence);
    ret.Text().SetTranslationHandler(handler);
    return ret;
}

WidgetsGlobalTableActionsScopeHandler WidgetsGlobalTableActionsScopeHandlers::AddHandler(const FTranslationHandler& handler, const QKeySequence& sequence, const class WidgetsGlobalTableActionId& globalActionId, const FAction& actionHandler)
{
    return AddHandler(handler, sequence, WidgetsGlobalTableActionsScope::GetInstance().FindAction(globalActionId), actionHandler);
}

WidgetsGlobalTableActionsScopeHandler WidgetsGlobalTableActionsScopeHandlers::FindHandler(const Latin1Name& id)
{
    auto* action = WidgetsGlobalTableActionsScope::GetInstance().FindAction(id);
    Q_ASSERT(action != nullptr);
    return Handlers[action];
}

WidgetsGlobalTableActionId::WidgetsGlobalTableActionId(const char* id, WidgetsGlobalTableActionsScope::EnableIfMode mode)
    : Super(id)
{
    if(WidgetsGlobalTableActionsScope::IsInitialized()) {
        WidgetsGlobalTableActionsScope::GetInstance().registerAction(*this, mode);
    } else {
        m_delayedRegistration.append(std::make_pair(*this, mode));
    }
}

IMPLEMENT_GLOBAL_ACTION(GlobalActionDebugSelectId, WidgetsGlobalTableActionsScope::EIM_Default)
IMPLEMENT_GLOBAL_ACTION(GlobalActionDebugJSONId, WidgetsGlobalTableActionsScope::EIM_TableHasSelection)
IMPLEMENT_GLOBAL_ACTION(GlobalActionCopyId, WidgetsGlobalTableActionsScope::EIM_TableHasSelection)
IMPLEMENT_GLOBAL_ACTION(GlobalActionCopyWithHeadersId, WidgetsGlobalTableActionsScope::EIM_TableHasSelection)
IMPLEMENT_GLOBAL_ACTION(GlobalActionCutId, WidgetsGlobalTableActionsScope::EIM_TableHasSelectionAndIsEditable)
IMPLEMENT_GLOBAL_ACTION(GlobalActionPasteId, WidgetsGlobalTableActionsScope::EIM_TableIsEditable)
IMPLEMENT_GLOBAL_ACTION(GlobalActionDeleteId, WidgetsGlobalTableActionsScope::EIM_TableHasSelectionAndIsEditable)
IMPLEMENT_GLOBAL_ACTION(GlobalActionInsertId, WidgetsGlobalTableActionsScope::EIM_TableIsEditable)
IMPLEMENT_GLOBAL_ACTION(GlobalActionImportId, WidgetsGlobalTableActionsScope::EIM_TableIsEditable)
