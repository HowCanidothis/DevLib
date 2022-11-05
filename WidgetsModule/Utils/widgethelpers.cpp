#include "widgethelpers.h"

#include <QSortFilterProxyModel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTableView>
#include <QHeaderView>
#include <QClipboard>
#include <QCompleter>
#include <QCheckBox>
#include <QSplitter>
#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QKeyEvent>
#include <QAction>
#include <QGroupBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QColorDialog>
#include <QWidgetAction>
#include <QLabel>
#include <QScrollArea>
#include <QMenu>
#include <QSettings>
#include <QHBoxLayout>
#include <ActionsModule/internal.hpp>

#include <optional>

#include <ActionsModule/internal.hpp>

#include "WidgetsModule/Models/viewmodelsdefaultfieldmodel.h"
#include "WidgetsModule/Actions/widgetsglobaltableactionsscope.h"
#include "WidgetsModule/Managers/widgetsdialogsmanager.h"
#include "WidgetsModule/Managers/widgetsfocusmanager.h"
#include "WidgetsModule/Managers/widgetsstandardtableheadermanager.h"

#include "WidgetsModule/ProgressBars/mainprogressbar.h"

#include "WidgetsModule/TableViews/Header/widgetsresizableheaderattachment.h"

#include "WidgetsModule/Attachments/widgetsactivetableattachment.h"
#include "WidgetsModule/Attachments/tableviewwidgetattachment.h"
#include "WidgetsModule/Attachments/widgetslocationattachment.h"

#include "WidgetsModule/Utils/iconsmanager.h"

#include "WidgetsModule/Utils/styleutils.h"

Q_DECLARE_METATYPE(SharedPointer<LocalPropertyInt>)
Q_DECLARE_METATYPE(SharedPointer<DelayedCallObject>)
Q_DECLARE_METATYPE(SharedPointer<CommonDispatcher<const Name&>>)
Q_DECLARE_METATYPE(SharedPointer<Dispatcher>)
Q_DECLARE_METATYPE(SharedPointer<CommonDispatcher<qint32>>)

class WidgetsAttachment : public QObject
{
    using Super = QObject;
public:
    using FFilter = std::function<bool (QObject*, QEvent*)>;
    WidgetsAttachment(const FFilter& filter, QObject* parent);

    static void Attach(QObject* target, const FFilter& filter);

private:
    bool eventFilter(QObject* watched, QEvent* e) override;

protected:
    FFilter m_filter;
};

WidgetsAttachment::WidgetsAttachment(const FFilter& filter, QObject* parent)
    : Super(parent)
    , m_filter(filter)
{
    parent->installEventFilter(this);
}

void WidgetsAttachment::Attach(QObject* target, const std::function<bool (QObject*, QEvent*)>& filter)
{
    new WidgetsAttachment(filter, target);
}

bool WidgetsAttachment::eventFilter(QObject* watched, QEvent* e)
{
    return m_filter(watched, e);
}

class WidgetsDisconnectableAttachment : public WidgetsAttachment
{
    using Super = WidgetsAttachment;
public:
    using Super::Super;

    static void Attach(QObject* target, const FFilter& filter);

private:
    bool eventFilter(QObject* watched, QEvent* e) override;
};

void WidgetsDisconnectableAttachment::Attach(QObject* target, const std::function<bool (QObject*, QEvent*)>& filter)
{
    new WidgetsDisconnectableAttachment(filter, target);
}

bool WidgetsDisconnectableAttachment::eventFilter(QObject*, QEvent* e)
{
    return m_filter(this, e);
}

const WidgetLineEditWrapper& WidgetLineEditWrapper::SetDynamicSizeAdjusting() const
{
    auto* edit = GetWidget();
    auto invalidate = [edit]{
        QFontMetrics fm(edit->font());
        int pixelsWide = fm.width(edit->text());
        pixelsWide += edit->contentsMargins().left() + edit->contentsMargins().right() + 20;
        edit->setMinimumWidth(pixelsWide);
    };
    QObject::connect(edit, &QLineEdit::textChanged, invalidate);
    invalidate();
    return *this;
}

LocalPropertyBool& WidgetLineEditWrapper::WidgetReadOnly() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_readonly", [](QObject* object, const LocalPropertyBool& readOnly){
        auto* line = reinterpret_cast<QLineEdit*>(object);
        line->setReadOnly(readOnly);
        StyleUtils::UpdateStyle(line);
    }, GetWidget()->isReadOnly());
}

void WidgetsObserver::EnableAutoCollapsibleGroupboxes()
{
    OnAdded += { this, [](QWidget* widget){
        auto* groupBox = qobject_cast<QGroupBox*>(widget);
        if(groupBox != nullptr && groupBox->isCheckable()) {
            WidgetGroupboxWrapper(groupBox).AddCollapsing();
        }
    } };
}

WidgetsObserver::WidgetsObserver()
{
    qApp->installEventFilter(this);
}

WidgetsObserver& WidgetsObserver::GetInstance()
{
    static WidgetsObserver res;
    return res;
}

bool WidgetsObserver::eventFilter(QObject* o, QEvent *e)
{
    if(o->isWidgetType() && e->type() == QEvent::Create) {
#ifdef QT_DEBUG
        Q_ASSERT(!o->property("a_command").toBool());
#endif
        auto callDelayed = DelayedCallObjectCreate();

        callDelayed->Call(CONNECTION_DEBUG_LOCATION, [this, o]{
            OnAdded(reinterpret_cast<QWidget*>(o));
            o->setProperty("a_command", false);
        });
        o->setProperty("a_command", QVariant::fromValue(callDelayed));
    }
    return false;
}

static const char* WidgetAppearanceAnimationPropertyName = "a_WAA";
static const char* WidgetAppearanceAnimationIsHidePropertyName = "a_WAAH";

Q_DECLARE_METATYPE(SharedPointer<QPropertyAnimation>)
Q_DECLARE_METATYPE(SharedPointer<QtLambdaConnections>)

WidgetsMatchingAttachment* WidgetTableViewWrapper::CreateMatching(QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns) const
{
    return new WidgetsMatchingAttachment(GetWidget(), targetModel, targetImportColumns);
}

WidgetDialogWrapper::WidgetDialogWrapper(const Name& id, const std::function<DescCustomDialogParams ()>& paramsCreator)
    : Super(WidgetsDialogsManager::GetInstance().GetOrCreateCustomDialog(id, paramsCreator))
{

}

void WidgetColorDialogWrapper::Show(const DescShowDialogParams& params) const
{
    WidgetsDialogsManager::GetInstance().ShowDialog(GetWidget(), params);
}

void WidgetDialogWrapper::Show(const DescShowDialogParams& params) const
{
    WidgetsDialogsManager::GetInstance().ShowDialog(GetWidget(), params);
}

QHeaderView* WidgetTableViewWrapper::InitializeHorizontal(const DescTableViewParams& params) const
{
    auto* tableView = GetWidget();
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(Qt::Horizontal, tableView);
    tableView->setHorizontalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto* columnsAction = dragDropHeader->CreateShowColumsMenu(params);
    tableView->setProperty("ColumnsAction", (size_t)columnsAction);

    if(params.UseStandardActions) {
        auto* editScope = ActionsManager::GetInstance().FindScope("TableEdit");
        if(editScope != nullptr){
            MenuWrapper tableViewWrapper(tableView);
            tableViewWrapper.AddGlobalTableAction(GlobalActionCopyId);
            tableViewWrapper.AddGlobalTableAction(GlobalActionCopyWithHeadersId);
            tableViewWrapper.AddSeparator();
            tableViewWrapper.AddGlobalTableAction(GlobalActionInsertId);
            tableViewWrapper.AddGlobalTableAction(GlobalActionPasteId);
            tableViewWrapper.AddGlobalTableAction(GlobalActionImportId);
            tableViewWrapper.AddSeparator();
            tableViewWrapper.AddGlobalTableAction(GlobalActionDeleteId);
            //tableViewWrapper.AddTableColumnsAction();
            tableView->addAction(columnsAction->menuAction());
        }
    }

    tableView->setWordWrap(true);
    auto* verticalHeader = tableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    WidgetsActiveTableViewAttachment::Attach(tableView);
    WidgetsStandardTableHeaderManager::GetInstance().Register(params, dragDropHeader);
    return dragDropHeader;
}

WidgetHeaderViewWrapper::WidgetHeaderViewWrapper(QHeaderView* header)
    : Super(header)
{

}

LocalPropertyBool& WidgetHeaderViewWrapper::SectionVisibility(qint32 logicalIndex) const
{
    return *GetOrCreateProperty<LocalPropertyBool>(QString("a_sectionVisibility_%1").arg(logicalIndex).toLatin1(), [logicalIndex](QObject* o, const LocalPropertyBool& value){
        auto* header = reinterpret_cast<QHeaderView*>(o);
        header->setSectionHidden(logicalIndex, !value);
    });
}

const WidgetHeaderViewWrapper& WidgetHeaderViewWrapper::MoveSection(qint32 logicalIndexFrom, qint32 logicalIndexTo) const
{
    auto* header = GetWidget();
    header->moveSection(header->visualIndex(logicalIndexFrom), header->visualIndex(logicalIndexTo));
    return *this;
}

QHeaderView* WidgetTableViewWrapper::InitializeVertical(const DescTableViewParams& params) const
{
    auto* tableView = GetWidget();
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(Qt::Vertical, tableView);
    tableView->setVerticalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto* columnsAction = dragDropHeader->CreateShowColumsMenu(params);
    tableView->setProperty("ColumnsAction", (size_t)columnsAction);
    if(params.UseStandardActions) {
        auto* editScope = ActionsManager::GetInstance().FindScope("TableEdit");
        if(editScope != nullptr){
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionCopyId);
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionCopyWithHeadersId);
            MenuWrapper(tableView).AddSeparator();
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionInsertId);
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionPasteId);
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionImportId);
            MenuWrapper(tableView).AddSeparator();
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionDeleteId);
            tableView->addAction(columnsAction->menuAction());
        }
    }

    tableView->setWordWrap(true);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    WidgetsActiveTableViewAttachment::Attach(tableView);
    WidgetsStandardTableHeaderManager::GetInstance().Register(params, dragDropHeader);
    return dragDropHeader;
}

WidgetPushButtonWrapper::WidgetPushButtonWrapper(QPushButton* pushButton)
    : Super(pushButton)
{

}

LocalPropertyBool& WidgetPushButtonWrapper::WidgetChecked() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_checked", [](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QPushButton*>(object);
        action->setChecked(visible);
    }, false);
}

const WidgetPushButtonWrapper& WidgetPushButtonWrapper::SetIcon(const Name& iconId) const
{
    GetWidget()->setIcon(IconsManager::GetInstance().GetIcon(iconId));
    return *this;
}

Dispatcher& ActionWrapper::OnClicked() const
{
    auto* widget = GetWidget();
    return *Injected<Dispatcher>("a_on_clicked", [widget]{
        auto* result = new Dispatcher();
        widget->connect(widget, &QAction::triggered, [result]{ result->Invoke(); });
        return result;
    });
}

const WidgetPushButtonWrapper& WidgetPushButtonWrapper::SetControl(ButtonRole i, bool update) const
{
    if(update) {
        StyleUtils::ApplyStyleProperty("a_control", GetWidget(), (qint32)i);
    } else {
        GetWidget()->setProperty("a_control", (qint32)i);
    }
    return *this;
}

TranslatedStringPtr WidgetPushButtonWrapper::WidgetText() const
{
    auto* label = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_text", [label](QObject*, const LocalPropertyString& text){
        label->setText(text);
    });
}

QList<int> WidgetTableViewWrapper::SelectedRowsSorted() const
{
    auto ret = SelectedRowsSet().toList();
    std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QList<int> WidgetTableViewWrapper::SelectedColumnsSorted() const
{
    auto ret = SelectedColumnsSet().toList();
	std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QSet<int> WidgetTableViewWrapper::SelectedRowsSet() const
{
	QSet<int> set;
    auto selectedIndexes = GetWidget()->selectionModel()->selectedIndexes();

    for(const auto& index : selectedIndexes){
        set.insert(index.row());
    }

    QAbstractItemModel* model = GetWidget()->model();
    while(qobject_cast<QSortFilterProxyModel*>(model)) {
        model = reinterpret_cast<QSortFilterProxyModel*>(model)->sourceModel();
    }
    if(model->property(ExtraFieldsCountPropertyName) == 1) {
        set.remove(model->rowCount() - 1);
    }

    return set;
}

QSet<int> WidgetTableViewWrapper::SelectedColumnsSet() const
{
    QSet<int> set;
    auto selectedIndexes = GetWidget()->selectionModel()->selectedIndexes();

    for(const auto& index : selectedIndexes){
        set.insert(index.column());
    }
    return set;
}

void WidgetTableViewWrapper::SelectRowsAndScrollToFirst(const QSet<qint32>& rowIndices) const
{
    auto* table = GetWidget();
    table->clearSelection();
    if(rowIndices.isEmpty()) {
        return;
    }

    auto* model = table->model();
    auto* selection = table->selectionModel();
    std::optional<qint32> firstIndex;
    for(int r=0; r<model->rowCount(); ++r){
        if(rowIndices.contains(r)) {
            selection->select(model->index(r,0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            if(!firstIndex.has_value()) {
                firstIndex = r;
            }
        }
    }
    if(firstIndex.has_value()) {
        table->scrollTo(model->index(firstIndex.value(), 0));
    }
}

void WidgetTableViewWrapper::SelectColumnsAndScrollToFirst(const QSet<qint32>& columnIndices) const
{
    auto* table = GetWidget();
    table->clearSelection();
    if(columnIndices.isEmpty()) {
        return;
    }

    auto* model = table->model();
    auto* selection = table->selectionModel();
    std::optional<qint32> firstIndex;
    for(int c=0; c<model->columnCount(); ++c){
        if(columnIndices.contains(c)) {
            selection->select(model->index(0, c), QItemSelectionModel::Select | QItemSelectionModel::Columns);
            if(!firstIndex.has_value()) {
                firstIndex = c;
            }
        }
    }
    if(firstIndex.has_value()) {
        table->scrollTo(model->index(0, firstIndex.value()));
    }
}

WidgetTableViewWrapper::WidgetTableViewWrapper(QTableView* tableView)
    : WidgetWrapper(tableView)
{}

bool WidgetTableViewWrapper::CopySelectedTableContentsToClipboard(bool includeHeaders) const
{
    auto* tableView = GetWidget();
    auto selectedIndexes = tableView->selectionModel()->selectedIndexes();
    if(selectedIndexes.isEmpty()) {
        return false;
    }

    auto* header = tableView->horizontalHeader();
    std::sort(selectedIndexes.begin(), selectedIndexes.end(), [header](const QModelIndex& f, const QModelIndex& s) {
        if(f.row() == s.row()) {
            return header->visualIndex(f.column()) < header->visualIndex(s.column());
        }

        return f.row() < s.row();
    });

    QString text;
    if(includeHeaders) {
        auto rowIndex = selectedIndexes.first().row();
        for(const auto& index : selectedIndexes) {
            if(header->isSectionHidden(index.column())) {
                continue;
            }
            if(rowIndex != index.row()) {
                text += "\n";
                break;
            }
            text += tableView->model()->headerData(index.column(), Qt::Horizontal).toString() + "\t";
        }
    }

    auto rowIndex = selectedIndexes.first().row();
    for(const auto& index : selectedIndexes) {
        if(header->isSectionHidden(index.column())) {
            continue;
        }
        if(rowIndex != index.row()) {
            rowIndex = index.row();
            if(!text.isEmpty()) {
                text.resize(text.size() - 1);
            }
            text += "\n";
        }
        text += index.data().toString() + "\t";
    }
    if(!text.isEmpty()) {
        text.resize(text.size() - 1);
    }

    QClipboard* clipboard = qApp->clipboard();
    clipboard->setText(text);
    return true;
}

// NOTE. Model must be case insensitively sorted! Otherwise there'll be undefined completion behavior
QCompleter* WidgetComboboxWrapper::CreateCompleter(QAbstractItemModel* model, const std::function<void (const QModelIndex& index)>& onActivated, qint32 column) const
{
    auto* combo = GetWidget();
    combo->setModel(model);
    combo->setEditable(true);
    auto* completer = new QCompleter(combo);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionColumn(column);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setModel(model);
    if(onActivated != nullptr) {
        completer->connect(completer, QOverload<const QModelIndex&>::of(&QCompleter::activated), [onActivated](const QModelIndex& index){
            onActivated(index);
        });
        combo->connect(combo, QOverload<qint32>::of(&QComboBox::activated), [combo, onActivated](qint32 row){
            onActivated(combo->model()->index(row, 0));
        });
    }
    combo->setModelColumn(column);
    combo->setCompleter(completer);
    DisconnectModel();
    return completer;
}


const WidgetComboboxWrapper& WidgetComboboxWrapper::DisconnectModel() const
{
    auto* combo = GetWidget();
    combo->setInsertPolicy(QComboBox::NoInsert);
    auto* viewModel = combo->model();
    QObject::disconnect(viewModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                       combo, SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
    QObject::disconnect(viewModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
               combo, SLOT(_q_updateIndexBeforeChange()));
    QObject::disconnect(viewModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
               combo, SLOT(_q_rowsInserted(QModelIndex,int,int)));
    QObject::disconnect(viewModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
               combo, SLOT(_q_updateIndexBeforeChange()));
    QObject::disconnect(viewModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
               combo, SLOT(_q_rowsRemoved(QModelIndex,int,int)));
    QObject::disconnect(viewModel, SIGNAL(destroyed()),
               combo, SLOT(_q_modelDestroyed()));
    QObject::disconnect(viewModel, SIGNAL(modelAboutToBeReset()),
               combo, SLOT(_q_updateIndexBeforeChange()));
    QObject::disconnect(viewModel, SIGNAL(modelReset()),
               combo, SLOT(_q_modelReset()));
    return *this;
}

WidgetGroupboxWrapper::WidgetGroupboxWrapper(QGroupBox* groupBox)
    : WidgetWrapper(groupBox)
{}

WidgetComboboxWrapper::WidgetComboboxWrapper(QComboBox* combobox)
    : WidgetWrapper(combobox)
{}

const WidgetComboboxWrapper& WidgetComboboxWrapper::EnableStandardItems(const QSet<qint32>& indices) const
{
    auto* itemModel = qobject_cast<QStandardItemModel*>(GetWidget()->model());
    if(itemModel != nullptr){
        for(const auto& index : indices){
            auto* item = itemModel->item(index);
            item->setFlags(item->flags().setFlag(Qt::ItemIsEnabled, true));
        }
    } else {
        Q_ASSERT(false);
    }
    return *this;
}

bool WidgetComboboxWrapper::SetCurrentData(const QVariant& value, Qt::ItemDataRole role) const
{
    auto indexOf = ViewModelWrapper(GetWidget()->model()).IndexOf([&](const QModelIndex& index){
        return index.data(role) == value;
    });
    if(indexOf != -1) {
        GetWidget()->setCurrentIndex(indexOf);
        return true;
    }
    return false;
}

CommonDispatcher<qint32>& WidgetComboboxWrapper::OnActivated() const
{
    auto* widget = GetWidget();
    return *Injected<CommonDispatcher<qint32>>("a_on_activated", [widget]{
        auto* result = new CommonDispatcher<qint32>();
        widget->connect(widget, QOverload<qint32>::of(&QComboBox::activated), [result](int index){
            result->Invoke(index);
        });
        return result;
    });
}

void WidgetWrapper::Highlight(qint32 unhightlightIn) const
{
    StyleUtils::ApplyStyleProperty("w_highlighted", GetWidget(), true);

    if(unhightlightIn > 0) {
        auto wrapper = *this;
        QTimer::singleShot(unhightlightIn, [wrapper]{
            wrapper.Lowlight();
        });
    }
}

Dispatcher& WidgetPushButtonWrapper::OnClicked() const
{
    auto* widget = GetWidget();
    return *Injected<Dispatcher>("a_on_clicked", [widget]{
        auto* result = new Dispatcher();
        widget->connect(widget, &QPushButton::clicked, [result]{ result->Invoke(); });
        return result;
    });
}

CommonDispatcher<const Name&>& WidgetLabelWrapper::OnLinkActivated() const
{
    auto* widget = GetWidget();
    return *Injected<CommonDispatcher<const Name&>>("a_on_link_activated", [widget]{
        auto* result = new CommonDispatcher<const Name&>();
        widget->connect(widget, &QLabel::linkActivated, [result](const QString& link){
            (*result)(Name(link));
        });
        return result;
    });
}

class WidgetsLocationAttachment* WidgetWrapper::LocateToParent(const DescWidgetsLocationAttachmentParams& params) const
{
    Q_ASSERT(Location() == nullptr);
    return InjectedWidget<WidgetsLocationAttachment>("a_location", [params](QWidget* parent){ return new WidgetsLocationAttachment(parent, params); });
}

WidgetsLocationAttachment* WidgetWrapper::Location() const
{
    return InjectedWidget<WidgetsLocationAttachment>("a_location", [](QWidget*){ return nullptr; });
}

void WidgetWrapper::Lowlight() const
{
    StyleUtils::ApplyStyleProperty("w_highlighted", GetWidget(), false);
}

const WidgetWrapper& WidgetWrapper::AddModalProgressBar() const
{
    Q_ASSERT(GetWidget()->isWindow());
    auto* progressBar = new MainProgressBar(GetWidget(), Qt::Window | Qt::FramelessWindowHint);
    auto* widget = GetWidget();
    AddEventFilter([progressBar, widget](QObject*, QEvent* event){
        if(event->type() == QEvent::Close && progressBar->isVisible()) {
            event->ignore();
            widget->close();
            return true;
        }
        return false;
    });
#ifdef QT_DEBUG
    Q_ASSERT(!GetWidget()->property("a_progressBar").toBool());
    GetWidget()->setProperty("a_progressBar", true);
#endif
    return *this;
}

const WidgetWrapper& WidgetWrapper::FixUp() const
{
    ForeachChildWidget([](const WidgetWrapper& w){
        auto* combobox = qobject_cast<QComboBox*>(w.GetWidget());
        if(combobox != nullptr) {
            combobox->style()->polish(combobox);
        }
    });

    ForeachChildWidget([](QWidget* w){
        if(qobject_cast<QSpinBox*>(w) != nullptr || qobject_cast<QDoubleSpinBox*>(w) != nullptr || qobject_cast<QComboBox*>(w)) {
            WidgetWrapper(w).AddEventFilter([](QObject* watched, QEvent* event) {
                if(event->type() == QEvent::Wheel) {
                    auto* widget = qobject_cast<QWidget*>(watched);
                    if(widget != nullptr && !widget->hasFocus()) {
                        event->ignore();
                        return true;
                    }
                }
                return false;
            });
            w->setFocusPolicy(Qt::StrongFocus);
        }
    });

    return *this;
}

const WidgetComboboxWrapper& WidgetComboboxWrapper::DisableStandardItems(const QSet<qint32>& indices) const
{
    auto* itemModel = qobject_cast<QStandardItemModel*>(GetWidget()->model());
    if(itemModel != nullptr){
        for(const auto& index : indices){
            auto* item = itemModel->item(index);
            if(item != nullptr) {
                item->setFlags(item->flags().setFlag(Qt::ItemIsEnabled, false));
            }
        }
    } else {
        Q_ASSERT(false);
    }
    return *this;
}

const WidgetGroupboxWrapper& WidgetGroupboxWrapper::AddCollapsing() const
{
    auto* widget = GetWidget();
    auto update = [widget](bool visible){
        auto animation = WidgetWrapper(widget).Injected<QPropertyAnimation>("a_collapsing_animation", [&]{
            return new QPropertyAnimation(widget, "maximumSize");
        });
        animation->stop();

        auto fullSize = QSize(widget->maximumWidth(), widget->sizeHint().height());
        auto minSize = QSize(widget->maximumWidth(), 24);
        animation->setDuration(200);
        animation->setStartValue(!visible ? fullSize : minSize);
        animation->setEndValue(visible ? fullSize : minSize);
        animation->start();
    };
    QObject::connect(widget, &QGroupBox::toggled, update);
    ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[widget, update]{
        update(widget->isChecked());
    });
    return *this;
}

const WidgetGroupboxWrapper& WidgetGroupboxWrapper::AddCollapsingDispatcher(Dispatcher* updater, QScrollArea* area) const
{
    std::function<qint32 (QGroupBox*)> handler;
    if(area != nullptr) {
        handler = [area](QGroupBox* w) -> qint32 {
            auto wMargins = w->contentsMargins();
            auto lMargins = w->layout()->contentsMargins();
            return wMargins.top() + wMargins.bottom() + lMargins.top() + lMargins.bottom() + area->widget()->sizeHint().height();
        };
    } else {
        handler = [](QGroupBox* w) -> qint32 { return w->sizeHint().height(); };
    }
    auto collapsingData = InjectedCommutator("a_collapsing", [handler](QObject* w) {
                                 auto* widget = reinterpret_cast<QGroupBox*>(w);
                                 if(widget->isChecked()) {
                                     ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[widget, handler]{
                                         widget->setMaximumSize(QSize(widget->maximumWidth(), handler(widget)));
                                     });
                                 }
                             });
    collapsingData->Commutator.ConnectFrom(CONNECTION_DEBUG_LOCATION, *updater).MakeSafe(collapsingData->Connections);
    return *this;
}

WidgetWrapper::WidgetWrapper(QWidget* widget)
    : Super(widget)
{

}

TranslatedStringPtr WidgetWrapper::WidgetToolTip() const
{
    auto* widget = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_tooltip", [widget](QObject*, const TranslatedString& text){
        widget->setToolTip(text);
    });
}

DispatcherConnection WidgetWrapper::ConnectEnablityFrom(const char* conInfo, QWidget* widget) const
{
    return WidgetEnablity().ConnectFrom(conInfo, WidgetWrapper(widget).WidgetEnablity());
}

DispatcherConnection WidgetWrapper::ConnectVisibilityFrom(const char* conInfo, QWidget* widget) const
{
    return WidgetVisibility().ConnectFrom(conInfo, WidgetWrapper(widget).WidgetVisibility());
}

DispatcherConnection WidgetWrapper::ConnectEnablityTo(const char* conInfo, QWidget* widget) const
{
    return WidgetWrapper(widget).WidgetEnablity().ConnectFrom(conInfo, WidgetEnablity());
}

DispatcherConnection WidgetWrapper::ConnectVisibilityTo(const char* conInfo, QWidget* widget) const
{
    return WidgetWrapper(widget).WidgetVisibility().ConnectFrom(conInfo, WidgetVisibility());
}

DispatcherConnections WidgetWrapper::CreateVisibilityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const
{
    auto result = createRule<WidgetWrapper>(debugLocation, QOverload<>::of(&WidgetWrapper::WidgetVisibility), handler, additionalWidgets, *dispatchers.first());
    for(auto* dispatcher : adapters::withoutFirst(dispatchers)) {
        result += WidgetVisibility().ConnectFrom(debugLocation, handler, *dispatcher);
    }
    return result;
}

DispatcherConnections WidgetWrapper::CreateEnablityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const
{
    auto result = createRule<WidgetWrapper>(debugLocation, &WidgetWrapper::WidgetEnablity, handler, additionalWidgets, *dispatchers.first());
    for(auto* dispatcher : adapters::withoutFirst(dispatchers)) {
        result += WidgetEnablity().ConnectFrom(debugLocation, handler, *dispatcher);
    }
    return result;
}

void WidgetWrapper::ActivateWindow(int mode, qint32 delay) const
{
    StyleUtils::ApplyStyleProperty("w_showfocus", GetWidget(), mode);
    auto* layout = GetWidget()->layout();
    FAction revertMargins = []{};
    if(layout != nullptr) {
        auto margins = layout->contentsMargins();
        auto minFrame = SharedSettings::GetInstance().StyleSettings.ShowFocusMinFrame.Native();
        layout->setContentsMargins(std::max(margins.left(), minFrame), std::max(margins.top(), minFrame), std::max(margins.right(), minFrame), std::max(margins.bottom(), minFrame));
        revertMargins = [layout, margins]{
            layout->setContentsMargins(margins);
        };
    }
    Q_ASSERT(delay > 0);
    auto wrapper = *this;
    QTimer::singleShot(delay, [wrapper, revertMargins]{
        StyleUtils::ApplyStyleProperty("w_showfocus", wrapper.GetWidget(), 0);
        revertMargins();
    });
}

void WidgetWrapper::SetVisibleAnimated(bool visible, int duration, double opacity) const
{
    if(visible) {
        ShowAnimated(duration, opacity);
    } else {
        HideAnimated(duration);
    }
}

const WidgetWrapper& WidgetWrapper::BlockWheel() const
{
    AddEventFilter([](QObject*, QEvent* event){
        if(event->type() == QEvent::Wheel) {
            return true;
        }
        return false;
    });
    return *this;
}

WidgetLineEditWrapper::WidgetLineEditWrapper(class QLineEdit* lineEdit)
    : WidgetWrapper(lineEdit)
{

}

const WidgetWrapper& WidgetWrapper::AddEventFilter(const std::function<bool (QObject*, QEvent*)>& filter) const
{
    WidgetsAttachment::Attach(GetWidget(), filter);
    return *this;
}

const WidgetWrapper& WidgetWrapper::AddDisconnectableEventFilter(const std::function<bool (QObject*, QEvent*)>& filter) const
{
    WidgetsDisconnectableAttachment::Attach(GetWidget(), filter);
    return *this;
}

const WidgetWrapper& WidgetWrapper::AddToFocusManager(const QVector<QWidget*>& additionalWidgets) const
{
    auto* target = GetWidget();
    auto eventFilter = [target](QObject*, QEvent* event){
        switch (event->type()) {
        case QEvent::FocusIn: FocusManager::GetInstance().SetFocusWidget(target); break;
        default: break;
        }
        return false;
    };

    QObject::connect(GetWidget(), &QWidget::destroyed, [target]{
        FocusManager::GetInstance().destroyed(target);
    });
    AddEventFilter(eventFilter);
    ForeachChildWidget([eventFilter](const WidgetWrapper& widget){
        widget.AddEventFilter(eventFilter);
    });
    for(auto* child : additionalWidgets) {
        WidgetWrapper(child).AddEventFilter(eventFilter);
    }

    return *this;
}

LocalPropertyBool& WidgetWrapper::WidgetCollapsing(Qt::Orientation orientation, qint32 initialWidth) const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_collapsed", [orientation, initialWidth](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QWidget*>(object);
        QSize fullSize, minSize;
        if(orientation == Qt::Horizontal) {
            fullSize = QSize(initialWidth, action->maximumHeight());
            minSize = QSize(0, action->maximumHeight());
        } else {
            fullSize = QSize(action->maximumWidth(), initialWidth);
            minSize = QSize(action->maximumWidth(), 0);
        }
        auto animation = WidgetWrapper(action).Injected<QPropertyAnimation>("a_collapsedAnimation", [&]{
            return new QPropertyAnimation(action, "maximumSize");
        });
        animation->stop();
        animation->setDuration(250);
        animation->setEasingCurve(QEasingCurve::OutExpo);
        animation->setStartValue(!visible ? fullSize : minSize);
        animation->setEndValue(visible ? fullSize : minSize);
        animation->start();
    }, true);
}

void WidgetWrapper::ShowAnimated(int duration, double opacity) const
{
    auto* widget = GetWidget();
    auto prevAnimation = widget->property(WidgetAppearanceAnimationPropertyName).value<SharedPointer<QPropertyAnimation>>();
    if(prevAnimation != nullptr) {
        prevAnimation->stop();
    }

    auto& visible = WidgetWrapper(widget).WidgetVisibility();
    if(visible) {
        return;
    }

    auto prevOpacity = 0.0;
    auto prevEffect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if(prevEffect != nullptr) {
        prevOpacity = prevEffect->opacity();
    }

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    auto animation = ::make_shared<QPropertyAnimation>(effect,"opacity");
    widget->setProperty(WidgetAppearanceAnimationPropertyName, QVariant::fromValue(animation));
    animation->setDuration(duration);
    animation->setStartValue(prevOpacity);
    animation->setEndValue(opacity);
    animation->setEasingCurve(QEasingCurve::InBack);
    animation->start();
    visible = true;
}

void WidgetWrapper::HideAnimated(int duration) const
{
    auto* widget = GetWidget();
    auto prevAnimation = widget->property(WidgetAppearanceAnimationPropertyName).value<SharedPointer<QPropertyAnimation>>();
    if(prevAnimation != nullptr) {
        if(prevAnimation->property(WidgetAppearanceAnimationIsHidePropertyName).toBool()) {
            return;
        }
        prevAnimation->stop();
    }

    auto& visible = WidgetWrapper(widget).WidgetVisibility();
    if(!visible) {
        return;
    }

    auto prevOpacity = 1.0;
    auto prevEffect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if(prevEffect != nullptr) {
        prevOpacity = prevEffect->opacity();
    }

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    auto animation = ::make_shared<QPropertyAnimation>(effect,"opacity");
    widget->setProperty(WidgetAppearanceAnimationPropertyName, QVariant::fromValue(animation));
    animation->setDuration(duration);
    animation->setStartValue(prevOpacity);
    animation->setEndValue(0);
    animation->setProperty(WidgetAppearanceAnimationIsHidePropertyName, true);
    animation->setEasingCurve(QEasingCurve::OutBack);
    animation->start();
    animation->connect(animation.get(), &QPropertyAnimation::stateChanged, [widget](QAbstractAnimation::State newState, QAbstractAnimation::State){
        if(newState == QAbstractAnimation::Stopped) {
            WidgetWrapper(widget).WidgetVisibility() = false;
        }
    });
    visible = true;
}

const WidgetWrapper& WidgetWrapper::SetPalette(const QHash<qint32, LocalPropertyColor*>& palette) const
{
#ifdef QT_DEBUG
    Q_ASSERT(!GetWidget()->property("a_palette").toBool());
#endif
    auto connections = DispatcherConnectionsSafeCreate();
    auto updater = DelayedCallObjectCreate();
    auto* pWidget = GetWidget();
    auto update = updater->Wrap(CONNECTION_DEBUG_LOCATION, [pWidget, palette]{
        auto pal = pWidget->palette();
        for(auto it(palette.cbegin()), e(palette.cend()); it != e; ++it) {
            pal.setColor((QPalette::ColorRole)it.key(), *it.value());
        }
        pWidget->setPalette(pal);
    });

    AddEventFilter([updater, update, connections](QObject*, QEvent* e){
        if(e->type() == QEvent::StyleChange) {
            update();
        }
        return false;
    });

    for(const auto* color : palette) {
        color->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [update]{
            update();
        }).MakeSafe(*connections);
    }

    GetWidget()->setProperty("a_palette", true);
    return *this;
}

DispatcherConnectionsSafe& WidgetWrapper::WidgetConnections() const
{
    return *Injected<DispatcherConnectionsSafe>("a_connections");
}

LocalPropertyBool& WidgetWrapper::WidgetHighlighted() const
{
    auto wrapper = *this;
    return *GetOrCreateProperty<LocalPropertyBool>("a_highlighted", [wrapper](QObject*, const LocalPropertyBool& highlighted){
        if(highlighted) {
            wrapper.Highlight();
        } else {
            wrapper.Lowlight();
        }
    }, false);
}

LocalPropertyBool& WidgetWrapper::WidgetVisibility(bool animated) const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_visible", [animated](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QWidget*>(object);
        if(animated) {
            WidgetWrapper(action).SetVisibleAnimated(visible);
        } else {
            action->setVisible(visible);
        }
    }, GetWidget()->isVisible());
}

LocalPropertyBool& WidgetWrapper::WidgetEnablity() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_enable", [](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QWidget*>(object);
        action->setEnabled(visible);
    }, GetWidget()->isEnabled());
}

bool WidgetWrapper::HasParent(QWidget* parent) const
{
    bool result = false;
    ForeachParentWidget([&result, parent](const WidgetWrapper& gypoParent){
        if(gypoParent.GetWidget() == parent) {
            result = true;
            return true;
        }
        return false;
    });
    return result;
}

void WidgetWrapper::ForeachParentWidget(const std::function<bool(const WidgetWrapper&)>& handler) const
{
    auto* parent = GetWidget()->parentWidget();
    while(parent != nullptr) {
        if(handler(parent)) {
            break;
        }
        parent = parent->parentWidget();
    }
}

void WidgetWrapper::ForeachChildWidget(const std::function<void (const WidgetWrapper&)>& handler) const
{
    auto childWidgets = GetWidget()->findChildren<QWidget*>();
    for(auto* childWidget : childWidgets) {
        handler(childWidget);
    }
}

ActionWrapper::ActionWrapper(QAction* action)
    : Super(action)
{

}

const ActionWrapper& ActionWrapper::SetShortcut(const QKeySequence& keySequence) const
{
    GetWidget()->setShortcut(keySequence);
    return *this;
}

const ActionWrapper& ActionWrapper::SetText(const QString& text) const
{
    GetWidget()->setText(text);
    return *this;
}

const ActionWrapper& ActionWrapper::SetIcon(const Name& iconName) const
{
    GetWidget()->setIcon(IconsManager::GetInstance().GetIcon(iconName));
    return *this;
}

LocalPropertyBool& ActionWrapper::WidgetVisibility() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_visible", [](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QAction*>(object);
        action->setVisible(visible);
    }, true);
}

LocalPropertyBool& ActionWrapper::WidgetEnablity() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_enable", [](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QAction*>(object);
        action->setEnabled(visible);
    }, true);
}

TranslatedStringPtr ActionWrapper::WidgetText() const
{
    return GetOrCreateProperty<TranslatedString>("a_text", [](QObject* object, const TranslatedString& text){
        auto* action = reinterpret_cast<QAction*>(object);
        action->setText(text);
    }, TR_NONE);
}

ActionWrapper MenuWrapper::AddSeparator() const
{
    QAction *action = new QAction(GetWidget());
    action->setSeparator(true);
    GetWidget()->addAction(action);
    return action;
}

ActionWrapper MenuWrapper::AddAction(const QString& title, const std::function<void ()>& handle) const
{
    auto result = new QAction(title, GetWidget());
    result->connect(result, &QAction::triggered, handle);
    GetWidget()->addAction(result);
    return result;
}

ActionWrapper MenuWrapper::AddAction(const QString& title, const std::function<void (QAction*)>& handle) const
{
    auto result = new QAction(title, GetWidget());
    result->connect(result, &QAction::triggered, [handle, result]{
        handle(result);
    });
    GetWidget()->addAction(result);
    return result;
}

ActionWrapper MenuWrapper::AddTableColumnsAction() const
{
    auto* tableView = qobject_cast<QTableView*>(GetWidget());
    Q_ASSERT(tableView != nullptr);
    auto* action = ((QMenu*)tableView->property("ColumnsAction").toLongLong())->menuAction();
    Q_ASSERT(action != nullptr);
    tableView->addAction(action);
    return action;
}

const MenuWrapper& MenuWrapper::AddGlobalAction(const QString& path) const
{
    auto* action = ActionsManager::GetInstance().FindAction(path);
    Q_ASSERT(action != nullptr);
    GetWidget()->addAction(action);
    return *this;
}

const MenuWrapper& MenuWrapper::AddGlobalTableAction(const Latin1Name& id) const
{
    auto* action = WidgetsGlobalTableActionsScope::GetInstance().FindAction(id);
    Q_ASSERT(action != nullptr);
    GetWidget()->addAction(action);
    if(m_globalActionsHandlers != nullptr) {
        auto foundIt = m_globalActionsHandlers->Handlers.find(action);
        Q_ASSERT(foundIt != m_globalActionsHandlers->Handlers.end());
        foundIt.value().SetVisible(true);
    }
    return *this;
}

ActionWrapper MenuWrapper::AddCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler) const
{
    auto result = new QAction(title, GetWidget());
    result->setCheckable(true);
    result->setChecked(checked);
    result->connect(result, &QAction::triggered, [handler, result]{
        handler(result->isChecked());
    });
    GetWidget()->addAction(result);
    return result;
}

WidgetColorDialogWrapper::WidgetColorDialogWrapper(class QColorDialog* dialog)
    : Super(dialog)
{}

const WidgetColorDialogWrapper& WidgetColorDialogWrapper::SetDefaultLabels() const
{
    static const QVector<std::function<void (QPushButton*)>> buttonsDelegates {
        [](QPushButton* btn) { btn->setText(QObject::tr("PICK SCREEN COLOR")); },
        [](QPushButton* btn) { btn->setText(QObject::tr("ADD TO CUSTOM COLORS")); },
        [](QPushButton* btn) { btn->setText(QObject::tr("APPLY")); },
        [](QPushButton* btn) { btn->setText(QObject::tr("CANCEL")); }
    };
    auto it = buttonsDelegates.cbegin(), e = buttonsDelegates.cend();
    ForeachChildWidget([&](const WidgetWrapper& widget) {
        auto* button = qobject_cast<QPushButton*>(widget);
        if(button != nullptr && it != e) {
            (*it)(button);
            ++it;
        }
    });
    return *this;
}

ActionWrapper MenuWrapper::AddColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler) const
{
    static QPixmap pixmap(10,10);
    auto colorAction = AddAction(title, [handler, color](QAction* action){
        WidgetColorDialogWrapper dialog(WidgetsDialogsManager::GetInstance().GetOrCreateDialog<QColorDialog>("ColorDialog", []{
            return WidgetColorDialogWrapper(new QColorDialog(WidgetsDialogsManager::GetInstance().GetParentWindow()))
                   .SetDefaultLabels();
        }, "ColorDialog"));
        dialog->setCurrentColor(color);
        dialog.Show(DescShowDialogParams());
        if(dialog->result() == QDialog::Accepted) {
            auto result = dialog->currentColor();
            pixmap.fill(result);
            action->setIcon(pixmap);
            handler(result);
        }
    });
    pixmap.fill(color);
    colorAction->setIcon(pixmap);
    return colorAction;
}

ActionWrapper MenuWrapper::AddDoubleAction(const QString& title, double value, const std::function<void (double value)>& handler) const
{
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout();
    layout->setContentsMargins(6,3,6,3);
    widget->setLayout(layout);
    auto* label = new QLabel(title);
    auto* spinBox = new QDoubleSpinBox;
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    layout->addWidget(spinBox);
    spinBox->setValue(value);
    QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [handler](double value) {
        handler(value);
    });
    auto* action = new QWidgetAction(GetWidget());
    action->setDefaultWidget(widget);
    GetWidget()->addAction(action);
    return action;
}

class PreventedFromClosingMenu : public QMenu
{
    using Super = QMenu;
public:
    using Super::Super;

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        QAction *action = activeAction();
        if (action && action->isEnabled()) {
            action->trigger();
        }
        else
            QMenu::mouseReleaseEvent(e);
    }
};

QMenu* MenuWrapper::CreatePreventedFromClosingMenu(const QString& title)
{
    return new PreventedFromClosingMenu(title);
}

QMenu* MenuWrapper::AddPreventedFromClosingMenu(const QString& title) const
{
    auto* result = new PreventedFromClosingMenu(title, GetWidget());
    GetWidget()->addAction(result->menuAction());
    return result;
}

QMenu* MenuWrapper::AddMenu(const QString& label) const
{
    auto* result = new QMenu(label, GetWidget());
    GetWidget()->addAction(result->menuAction());
    return result;
}

WidgetLabelWrapper::WidgetLabelWrapper(QLabel* label)
    : WidgetWrapper(label)
{}

TranslatedStringPtr WidgetLabelWrapper::WidgetText() const
{
    auto* label = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_text", [label](QObject*, const LocalPropertyString& text){
        label->setText(text);
    });
}

const WidgetWrapper& WidgetWrapper::CreateCustomContextMenu(const std::function<void (QMenu*)>& creatorHandler, bool preventFromClosing) const
{
    auto* w = GetWidget();
    w->setContextMenuPolicy(Qt::CustomContextMenu);
    auto connections = ::make_shared<QtLambdaConnections>();
    w->setProperty("a_customContextMenu", QVariant::fromValue(connections));
    connections->connect(GetWidget(), &QWidget::customContextMenuRequested, [creatorHandler, w, preventFromClosing](const QPoint& pos) {
        auto menu = preventFromClosing ? ::make_scoped<PreventedFromClosingMenu>() : ::make_scoped<QMenu>();
        creatorHandler(menu.get());
        menu->exec(w->mapToGlobal(pos));
    });
    return *this;
}

WidgetSpinBoxWrapper::WidgetSpinBoxWrapper(QSpinBox* widget)
    : Super(widget)
{}

LocalPropertyInt& WidgetSpinBoxWrapper::WidgetValue() const
{
    return *Injected<LocalPropertyInt>("a_value", [&]() -> LocalPropertyInt* {
       auto* property = new LocalPropertyInt();
       auto* widget = GetWidget();
       property->OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget, property]{ widget->setValue(*property); });
       widget->connect(widget, QOverload<qint32>::of(&QSpinBox::valueChanged), [property](qint32 value){
           *property = value;
       });
       property->SetSetterHandler(ThreadHandlerMain);
       return property;
   });
}

WidgetDoubleSpinBoxWrapper::WidgetDoubleSpinBoxWrapper(QDoubleSpinBox* widget)
    : Super(widget)
{}

LocalPropertyDouble& WidgetDoubleSpinBoxWrapper::WidgetValue() const
{
    return *Injected<LocalPropertyDouble>("a_value", [&]() -> LocalPropertyDouble* {
       auto* property = new LocalPropertyDouble();
       auto* widget = GetWidget();
       property->EditSilent() = widget->value();
       widget->connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [property](double value){
           *property = value;
       });
       property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [widget, property]{ widget->setValue(*property); });
       property->SetSetterHandler(ThreadHandlerMain);
       return property;
   });
}

LocalPropertyBool& WidgetDoubleSpinBoxWrapper::WidgetReadOnly() const
{
    return *Injected<LocalPropertyBool>("a_readOnly", [&]() -> LocalPropertyBool* {
        auto* property = new LocalPropertyBool();
        auto* widget = GetWidget();
        property->EditSilent() = widget->isReadOnly();
        property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [widget, property]{
            widget->setReadOnly(*property);
            StyleUtils::UpdateStyle(widget);
        });
        property->SetSetterHandler(ThreadHandlerMain);
        return property;
    });
}

WidgetSplitterWrapper::WidgetSplitterWrapper(QSplitter* splitter)
    : Super(splitter)
{}

void WidgetSplitterWrapper::SetWidgetSize(QWidget* widget, qint32 size) const
{
    auto index = GetWidget()->indexOf(widget);
    if(index != -1) {
        auto sizes = GetWidget()->sizes();
        sizes[index] = size;
        GetWidget()->setSizes(sizes);
    }
}

WidgetCheckBoxWrapper::WidgetCheckBoxWrapper(QCheckBox* target)
    : Super(target)
{}

LocalPropertyBool& WidgetCheckBoxWrapper::WidgetChecked() const
{
    auto* widget = GetWidget();
    return *GetOrCreateProperty<LocalPropertyBool>("a_checked",[widget](QObject*, const LocalPropertyBool& value){
        widget->setChecked(value);
    }, false);
}

TranslatedStringPtr WidgetCheckBoxWrapper::WidgetText() const
{
    auto* label = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_text", [label](QObject*, const LocalPropertyString& text){
        label->setText(text);
    });
}

ViewModelWrapper::ViewModelWrapper(QAbstractItemModel* model)
    : Super(model)
{}

const ViewModelWrapper& ViewModelWrapper::ForeachModelIndex(const QModelIndex& parent, const FIterationHandler& function) const
{
    auto* viewModel = GetViewModel();
    auto rowCount = viewModel->rowCount(parent);
    for(int r = 0; r < rowCount; ++r) {
        QModelIndex index = viewModel->index(r, 0, parent);
        if(function(index)) {
            return *this;
        }
        if( viewModel->hasChildren(index) ) {
            ForeachModelIndex(index, function);
        }
    }
    return *this;
}

qint32 ViewModelWrapper::IndexOf(const FIterationHandler& handler) const
{
    qint32 result = -1;
    ForeachModelIndex([&](const QModelIndex& index) {
        if(handler(index)) {
            result = index.row();
            return true;
        }
        return false;
    });
    return result;
}

ViewModelsDefaultFieldModel* ViewModelWrapper::CreateDefaultFieldModel(QObject* parent, const FTranslationHandler& field) const
{
    static const auto defaultField = TR(tr("None"));
    auto resField = field == nullptr ? defaultField : field;
    auto* model = new ViewModelsDefaultFieldModel(parent);
    model->SetSourceModel(GetViewModel());
    model->SetDefaultFieldLabel(resField);
    return model;
}

Dispatcher& ViewModelWrapper::OnReset() const
{
    auto* model = GetViewModel();
    return *Injected<Dispatcher>("a_onReset", [model]{
        auto* result = new Dispatcher();
        model->connect(model, &QAbstractItemModel::modelReset, [result]{ result->Invoke(); });
        return result;
    });
}
