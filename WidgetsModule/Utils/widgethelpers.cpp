#include "widgethelpers.h"

#include <QSortFilterProxyModel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTableView>
#include <QHeaderView>
#include <QClipboard>
#include <QCompleter>
#include <QListView>
#include <QCheckBox>
#include <QSplitter>
#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QKeyEvent>
#include <QScrollBar>
#include <QAction>
#include <QGroupBox>
#include <QPushButton>
#include <QToolButton>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QColorDialog>
#include <QWidgetAction>
#include <QLabel>
#include <QTextEdit>
#include <QScrollArea>
#include <QMenu>
#include <QSettings>
#include <QHBoxLayout>

#include <optional>

#include "WidgetsModule/Dialogs/widgetsdebugjsondialog.h"

#include "WidgetsModule/Models/viewmodelsdefaultfieldmodel.h"
#include "WidgetsModule/Models/modelslistbase.h"

#include "WidgetsModule/Actions/actionsmanager.h"
#include "WidgetsModule/Actions/widgetsglobaltableactionsscope.h"

#include "WidgetsModule/Managers/widgetsdialogsmanager.h"
#include "WidgetsModule/Managers/widgetsfocusmanager.h"
#include "WidgetsModule/Managers/widgetsstandardtableheadermanager.h"

#include "WidgetsModule/ProgressBars/mainprogressbar.h"
#include "WidgetsModule/ProgressBars/progressbar.h"

#include "WidgetsModule/TableViews/Header/widgetsresizableheaderattachment.h"

#include "WidgetsModule/Attachments/widgetsactivetableattachment.h"
#include "WidgetsModule/Attachments/tableviewwidgetattachment.h"
#include "WidgetsModule/Attachments/widgetslocationattachment.h"

#include "WidgetsModule/Widgets/elidedlabel.h"
#include "WidgetsModule/Widgets/Layouts/widgetsgroupboxlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetscomboboxlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetsdoublespinboxlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetslineeditlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetsspinboxlayout.h"
#include "WidgetsModule/Widgets/Layouts/widgetstabbarlayout.h"
#include "WidgetsModule/Widgets/widgetsspinboxwithcustomdisplay.h"

#include "WidgetsModule/Utils/iconsmanager.h"

#include "WidgetsModule/Utils/widgetstyleutils.h"
#include "WidgetsModule/Delegates/delegates.h"
#include <UnitsModule/internal.hpp>

using FCurrentChanged = SharedPointer<CommonDispatcher<qint32,qint32>>;

Q_DECLARE_METATYPE(SharedPointer<bool>)
Q_DECLARE_METATYPE(SharedPointer<CommonDispatcher<const QString&>>)
Q_DECLARE_METATYPE(SharedPointer<LocalPropertyString>)
Q_DECLARE_METATYPE(SharedPointer<DelayedCallObject>)
Q_DECLARE_METATYPE(SharedPointer<CommonDispatcher<const Name&>>)
Q_DECLARE_METATYPE(SharedPointer<Dispatcher>)
Q_DECLARE_METATYPE(SharedPointer<CommonDispatcher<qint32>>)
Q_DECLARE_METATYPE(SharedPointer<LocalPropertySequentialEnum<HighLightEnum>>)
Q_DECLARE_METATYPE(SharedPointer<LocalPropertyErrorsContainer>)
Q_DECLARE_METATYPE(FCurrentChanged)
Q_DECLARE_METATYPE(SP<QVector<QWidget*>>)
Q_DECLARE_METATYPE(SP<QCompleter>)

struct DisabledColumnComponentData
{
   bool Installed = false;
   QSet<qint32> DisabledItems;
};

Q_DECLARE_METATYPE(SharedPointer<DisabledColumnComponentData>)

EventFilterObject::EventFilterObject(const FFilter& filter, QObject* parent)
    : Super(parent)
    , m_filter(filter)
{
    parent->installEventFilter(this);
}

bool EventFilterObject::eventFilter(QObject* watched, QEvent* e)
{
    return m_filter(watched, e);
}

const MenuWrapper& MenuWrapper::SetOnContextMenu(const FAction& action) const
{
    OnContextMenu().Connect(CDL, action);
    return *this;
}

Dispatcher& MenuWrapper::OnContextMenu() const
{
    auto* w = GetWidget();
    return *Injected<Dispatcher>("a_onContextMenu", [w]{
        auto* result = new Dispatcher();
        if(auto* tv = qobject_cast<QTableView*>(w)) {
            WidgetWrapper(tv->viewport()).AddEventFilter([result](QObject*, QEvent* e) {
                if(e->type() == QEvent::ContextMenu) {
                    result->Invoke();
                }
                return false;
            });
        } else if(auto* menu = qobject_cast<QMenu*>(w)) {
            menu->connect(menu, &QMenu::aboutToShow, [result]{
                result->Invoke();
            });
        } else {
            WidgetWrapper(w).AddEventFilter([result](QObject*, QEvent* e){
                if(e->type() == QEvent::ContextMenu) {
                    result->Invoke();
                }
                return false;
            });
        }
        return result;
    });
}

const WidgetLineEditWrapper& WidgetLineEditWrapper::AddPasswordButton() const
{
    auto* le = GetWidget();
#ifdef QT_DEBUG
    Q_ASSERT(!le->property("HasBtnShowPassword").toBool());
    le->setProperty("HasBtnShowPassword", true);
#endif
    auto* eyeButton = new QPushButton(le);
    eyeButton->setObjectName("BtnShowPassword");
    eyeButton->setCheckable(true);
    le->setEchoMode(QLineEdit::Password);
    WidgetPushButtonWrapper(eyeButton).SetOnClicked([le, eyeButton]{
        if(eyeButton->isChecked()) {
            le->setEchoMode(QLineEdit::Normal);
        } else {
            le->setEchoMode(QLineEdit::Password);
        }
    });
    WidgetWrapper(eyeButton).LocateToParent(DescWidgetsLocationAttachmentParams(QuadTreeF::Location_MiddleRight).SetOffset({10,0}).DisableFullParentSize());
    return *this;
}

const WidgetLineEditWrapper& WidgetLineEditWrapper::AddCompleter(const QStringList& keys) const
{
    auto* te = GetWidget();

    Injected<QCompleter>("a_completer", [te, keys]{
        auto completer = new QCompleter(keys);
        completer->setWidget(te);

        WidgetWrapper(completer->popup()).AddEventFilter([completer](QObject*, QEvent* e) {
            if(completer->popup()->currentIndex().row() != -1 && e->type() == QEvent::KeyPress) {
                auto ke = static_cast<QKeyEvent*>(e);
                if(ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                    emit completer->activated(completer->popup()->currentIndex().data().toString());
                    return true;
                }
            }
            return false;
        });

        auto start = ::make_shared<qint32>(0);
        auto end = ::make_shared<qint32>(0);

        completer->connect(completer, QOverload<const QString&>::of(&QCompleter::activated), [completer, te, start,end](const QString& text){
            if(*start == -1) {
                return;
            }
            auto trueStart = *start;
            te->setText(te->text().replace(trueStart, *end - trueStart, text));
            te->setCursorPosition(trueStart + text.length());
            completer->popup()->hide();
        });

        te->connect(te, &QLineEdit::cursorPositionChanged, [start,end, te, completer](qint32, qint32 newPos){
            static QRegularExpression exp(R"((\$[\w\d\.]+))");

            QString result;
            auto match = exp.globalMatch(te->text());
            while(match.hasNext()) {
                auto next = match.next();
                *start = next.capturedStart();
                *end = next.capturedEnd();
                if(newPos >= *start && newPos <= *end) {
                    completer->setCompletionPrefix(next.captured(1));
                    if(completer->popup()->model()->rowCount() != 1 || completer->popup()->model()->index(0,0).data().toString() != next.captured(1)) {
                        completer->complete();
                    }
                    return;
                }
            }
            *start = -1;
            *end = -1;
            completer->popup()->hide();
        });
        return completer;
    });


    return *this;
}

const WidgetLineEditWrapper& WidgetLineEditWrapper::SetDynamicSizeAdjusting() const
{
    auto* edit = GetWidget();
    auto calcWidth = [edit] {
        QFontMetrics fm(edit->font());
        int pixelsWide = fm.width(edit->text());
        pixelsWide += edit->contentsMargins().left() + edit->contentsMargins().right() + 22;
        return pixelsWide;
    };
    auto invalidate = [edit, calcWidth]{
        auto res = calcWidth();
        if(!edit->hasFocus()) {
            edit->setMinimumWidth(res);
            edit->setMaximumWidth(res);
        }
    };
    auto animate = [edit, calcWidth](bool expand) {
        auto animation = WidgetWrapper(edit).Injected<QPropertyAnimation>("a_collapsing_animation", [&]{
            return new QPropertyAnimation(edit, "maximumSize");
        });
        auto width = calcWidth();
        animation->stop();

        auto fullSize = QSize(4000, edit->maximumHeight());
        auto minSize = QSize(width, edit->maximumHeight());
        animation->setDuration(200);
        animation->setStartValue(QSize(edit->width(), edit->maximumHeight()));
        animation->setEndValue(expand ? fullSize : minSize);
        animation->start();
    };
    QObject::connect(edit, &QLineEdit::textChanged, invalidate);
    WidgetWrapper(edit).AddEventFilter([edit, animate, invalidate](QObject*, QEvent* e) {
        switch(e->type()) {
        case QEvent::ShowToParent:
        case QEvent::StyleChange:
            invalidate();
            break;
        case QEvent::FocusIn:
            if(!edit->isReadOnly()) {
                animate(true);
            }
            break;
        case QEvent::FocusOut:
            if(!edit->isReadOnly()) {
                animate(false);
            }
            break;
        default: break;
        }

        return false;
    });
    return *this;
}

CommonDispatcher<const QString&>& WidgetLineEditWrapper::OnEditFinished() const
{
    auto* widget = GetWidget();
    return *Injected<CommonDispatcher<const QString&>>("a_on_ef", [widget]{
        auto* result = new CommonDispatcher<const QString&>();
        widget->connect(widget, &QLineEdit::editingFinished, [result, widget]{ result->Invoke(widget->text()); });
        return result;
    });
}

LocalPropertyString& WidgetLineEditWrapper::WidgetText() const
{
    Q_ASSERT(m_object->property("a_textec").isNull());
    return *GetOrCreateProperty<QLineEdit, LocalPropertyString>("a_text", [](QLineEdit* action, const LocalPropertyString& text){
        action->setText(text);
    }, [](QLineEdit* btn, LocalPropertyString* property){
        *property = btn->text();
    }, &QLineEdit::textChanged, [](QLineEdit*){}, GetWidget()->text());
}

LocalPropertyBool& WidgetLineEditWrapper::WidgetReadOnly() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_readonly", [](QObject* object, const LocalPropertyBool& readOnly){
        auto* line = reinterpret_cast<QLineEdit*>(object);
        line->setReadOnly(readOnly);
        WidgetWrapper(line).UpdateStyle();
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

CommonDispatcher<qint32, qint32>& WidgetTableViewWrapper::OnCurrentIndexChanged() const
{
    auto* w = GetWidget();
    Q_ASSERT(qobject_cast<QTableView*>(w));
    return *Injected<CommonDispatcher<qint32, qint32>>("a_on_current_index_changed", [w]{
        auto* result = new CommonDispatcher<qint32, qint32>();
        QObject::connect(w->selectionModel(), &QItemSelectionModel::currentChanged, [result](const QModelIndex& current, const QModelIndex& ) {
            result->Invoke(current.row(), current.column());
        });
        return result;
    });
}

const WidgetTableViewWrapper& WidgetTableViewWrapper::SetOnCurrentIndexChanged(const std::function<void (qint32, qint32)>& handler) const
{
    OnCurrentIndexChanged().Connect(CONNECTION_DEBUG_LOCATION, handler);
    return *this;
}

void WidgetWrapper::RegisterDialogView(const DescCustomDialogParams& params)
{

}

WidgetDialogWrapper::WidgetDialogWrapper(const Name& id, const std::function<DescCustomDialogParams ()>& paramsCreator)
    : Super(WidgetsDialogsManager::GetInstance().GetOrCreateDialog(id, paramsCreator))
{

}

qint32 WidgetDialogWrapper::Show(const DescShowDialogParams& params) const
{
    return WidgetsDialogsManager::GetInstance().ShowDialog(GetWidget(), params);
}

QHeaderView* WidgetTableViewWrapper::InitializeHorizontal(const DescTableViewParams& params) const
{
    auto* tableView = GetWidget();
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(Qt::Horizontal, tableView, params);
    tableView->setHorizontalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);

    EventFilterObject* focusConnector = WidgetWrapper(tableView).ConnectFocus(dragDropHeader);
    dragDropHeader->setFocusProxy(tableView);
    dragDropHeader->connect(dragDropHeader, &QWidget::destroyed, [focusConnector]{ delete focusConnector; });

    auto* columnsAction = WidgetsResizableHeaderAttachment::CreateShowColumsMenu(dragDropHeader, params);
    tableView->setProperty("ColumnsAction", (size_t)columnsAction);

    if(params.UseStandardActions) {
        auto* editScope = ActionsManager::GetInstance().FindScope("TableEdit");
        if(editScope != nullptr){
            MenuWrapper tableViewWrapper(tableView);
            tableViewWrapper.AddDebugActions();
            tableViewWrapper.AddGlobalTableAction(GlobalActionCopyId);
            tableViewWrapper.AddGlobalTableAction(GlobalActionCopyWithHeadersId);
            tableViewWrapper.AddGlobalTableAction(GlobalActionPasteId);
            tableViewWrapper.AddSeparator();
            tableViewWrapper.AddGlobalTableAction(GlobalActionInsertId);
            tableViewWrapper.AddGlobalTableAction(GlobalActionImportId);
            tableViewWrapper.AddSeparator();
            tableViewWrapper.AddGlobalTableAction(GlobalActionDeleteId);
            //tableViewWrapper.AddTableColumnsAction();
            tableView->addAction(columnsAction->menuAction());
        }
    }

    if(params.UseStandardActionHandlers) {
        WidgetTableViewWrapper(tableView).SetDefaultActionHandlers(false);
    }

#ifdef UNITS_MODULE_LIB
    auto* model = tableView->model();
    if(model != nullptr) {
        auto count = model->columnCount();
//        Q_ASSERT(count > 0);
        auto delegate = new DelegatesDoubleSpinBox(tableView);
        delegate->OnEditorAboutToBeShown.Connect(CONNECTION_DEBUG_LOCATION, [](class QDoubleSpinBox* editor, const QModelIndex& index){
            auto* unit = index.model()->headerData(index.column(), Qt::Horizontal, UnitRole).value<const Measurement*>();
            if(unit == nullptr) {
                return;
            }
            editor->setSingleStep(unit->CurrentStep);
            editor->setDecimals(unit->CurrentPrecision);
            auto min = index.data(MinLimitRole);
            auto max = index.data(MaxLimitRole);
            editor->setRange(min.isValid() ? min.toDouble() : std::numeric_limits<double>().lowest(), max.isValid() ? max.toDouble() : std::numeric_limits<double>().max());
        });
        for(int section(0); section < count; ++section){
            auto data = model->headerData(section, Qt::Horizontal, UnitRole);
            if(data.isValid()){
                tableView->setItemDelegateForColumn(section, delegate);
            }
        }
    }
#endif

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
    auto verticalHeader = tableView->verticalHeader();
    qint32 alignment = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap;
    verticalHeader->setDefaultAlignment((Qt::Alignment)alignment);
    verticalHeader->setStretchLastSection(params.StretchLastSection);
    verticalHeader->setSectionsMovable(params.SectionsMovable);
    verticalHeader->setSectionsClickable(params.SectionsClickable);
    verticalHeader->setHighlightSections(params.HighlightSections);
    verticalHeader->setDropIndicatorShown(params.DropIndicatorShown);
    verticalHeader->setSortIndicatorShown(params.SortIndicatorShown);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);

	EventFilterObject* focusConnector = WidgetWrapper(tableView).ConnectFocus(verticalHeader);
    verticalHeader->setFocusProxy(tableView);
    verticalHeader->connect(verticalHeader, &QWidget::destroyed, [focusConnector]{ delete focusConnector; });

    auto* columnsAction = WidgetsResizableHeaderAttachment::CreateShowColumsMenu(verticalHeader, params);
    tableView->setProperty("ColumnsAction", (size_t)columnsAction);
    if(params.UseStandardActions) {
        auto* editScope = ActionsManager::GetInstance().FindScope("TableEdit");
        if(editScope != nullptr){
            MenuWrapper(tableView).AddDebugActions();
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionCopyId);
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionCopyWithHeadersId);
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionPasteId);
            MenuWrapper(tableView).AddSeparator();
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionInsertId);
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionImportId);
            MenuWrapper(tableView).AddSeparator();
            MenuWrapper(tableView).AddGlobalTableAction(GlobalActionDeleteId);
            tableView->addAction(columnsAction->menuAction());
        }
    }

    if(params.UseStandardActionHandlers) {
        WidgetTableViewWrapper(tableView).SetDefaultActionHandlers(false);
    }

#ifdef UNITS_MODULE_LIB
    auto* model = tableView->model();
    if(model != nullptr) {
        auto count = model->rowCount();
        Q_ASSERT(count > 0);
        auto delegate = new DelegatesDoubleSpinBox(tableView);
        delegate->OnEditorAboutToBeShown.Connect(CONNECTION_DEBUG_LOCATION, [](class QDoubleSpinBox* editor, const QModelIndex& index){
            auto* unit = index.model()->headerData(index.row(), Qt::Vertical, UnitRole).value<const Measurement*>();
            if(unit == nullptr){
                return;
            }
            editor->setSingleStep(unit->CurrentStep);
            editor->setDecimals(unit->CurrentPrecision);
            auto min = index.data(MinLimitRole);
            auto max = index.data(MaxLimitRole);
            editor->setRange(min.isValid() ? min.toDouble() : std::numeric_limits<double>().lowest(), max.isValid() ? max.toDouble() : std::numeric_limits<double>().max());
        });
        for(int section(0); section < count; ++section){
            auto data = model->headerData(section, Qt::Vertical, UnitRole);
            if(data.isValid()){
                tableView->setItemDelegateForRow(section, delegate);
            }
        }
    }
#endif

    tableView->setWordWrap(true);
    WidgetsActiveTableViewAttachment::Attach(tableView);
//    WidgetsStandardTableHeaderManager::GetInstance().Register(params, verticalHeader);
    return verticalHeader;
}

WidgetTableViewColumnsAttachment* WidgetTableViewWrapper::AddColumnAttachment(const std::function<QWidget* (qint32)>& delegate) const
{
    auto* result = new WidgetTableViewColumnsAttachment(GetWidget());
    result->Initialize(delegate);
    return result;
}

WidgetAbstractButtonWrapper::WidgetAbstractButtonWrapper(QAbstractButton* button)
    : Super(button)
{

}

LocalPropertyBool& WidgetAbstractButtonWrapper::WidgetChecked() const
{
    auto* widget = GetWidget();
    return *Injected<LocalPropertyBool>("a_checked", [&]() -> LocalPropertyBool* {
        auto* property = new LocalPropertyBool(false);
        widget->setCheckable(true);
        property->ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget](bool value){
                                               widget->setChecked(value);
                                           });
        property->SetSetterHandler(ThreadHandlerMain);
        widget->connect(widget, &QAbstractButton::toggled, [widget, property](bool state){
            if(widget->property(WidgetProperties::ForceDisabled).toBool()) {
                widget->setChecked(*property);
                return;
            }
            *property = state;
            if(*property != state) {
                widget->setChecked(*property);
            }
        });
        return property;
    });
}

const WidgetAbstractButtonWrapper& WidgetAbstractButtonWrapper::SetIcon(const Name& iconId) const
{
    GetWidget()->setIcon(IconsManager::GetInstance().GetIcon(iconId));
    return *this;
}

const WidgetAbstractButtonWrapper& WidgetAbstractButtonWrapper::SetControl(ButtonRole i, bool update) const
{
    if(update) {
        ApplyStyleProperty("a_control", (qint32)i);
    } else {
        GetWidget()->setProperty("a_control", (qint32)i);
    }
    return *this;
}

TranslatedStringPtr WidgetAbstractButtonWrapper::WidgetText() const
{
    auto* label = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_text", [label](QObject*, const TranslatedString& text){
        label->setText(text.Native());
    });
}

Dispatcher& WidgetAbstractButtonWrapper::OnClicked() const
{
    auto* widget = GetWidget();
    return *Injected<Dispatcher>("a_on_clicked", [widget]{
        auto* result = new Dispatcher();
        widget->connect(widget, &QAbstractButton::clicked, [result]{ result->Invoke(); });
        return result;
    });
}

const ActionWrapper& ActionWrapper::ConnectEnablityFromViewModel(const char* cdl, ViewModelsTableBase* viewModel) const
{
    WidgetEnablity().ConnectFromDispatchers(cdl, [viewModel]{
        return viewModel->IsEnabled();
    }, viewModel->IsEditable, viewModel->ForceDisabled);

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

QList<int> WidgetTableViewWrapper::SelectedRowsSorted() const
{
    auto ret = SelectedRowsSet().values();
    std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QList<int> WidgetTableViewWrapper::SelectedColumnsSorted() const
{
    auto ret = SelectedColumnsSet().values();
	std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QSet<int> WidgetTableViewWrapper::SelectedRowsSet() const
{
	QSet<int> set;
    auto selectedIndexes = GetWidget()->selectionModel()->selectedIndexes();

    for(const auto& index : selectedIndexes){
        if(index.data(LastEditRowRole).toBool()) {
            continue;
        }
        set.insert(index.row());
    }

    QAbstractItemModel* model = GetWidget()->model();
    QAbstractItemModel* sourceModel = model;
    while(qobject_cast<QSortFilterProxyModel*>(sourceModel)) {
        sourceModel = reinterpret_cast<QSortFilterProxyModel*>(model)->sourceModel();
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
            auto indexToSelect = model->index(r,0);
            if(!firstIndex.has_value()) {
                firstIndex = r;
                selection->setCurrentIndex(indexToSelect, QItemSelectionModel::NoUpdate);
            }
            selection->select(indexToSelect, QItemSelectionModel::Select | QItemSelectionModel::Rows);
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

WidgetsGlobalTableActionsScopeHandlersPtr WidgetTableViewWrapper::SetDefaultActionHandlers(bool readOnly) const
{
    auto h = CreateDefaultActionHandlers();
    h->IsReadOnly = readOnly;
    h->ShowAll();
    return h;
}

WidgetsGlobalTableActionsScopeHandlersPtr WidgetTableViewWrapper::CreateDefaultActionHandlers() const
{
    return WidgetsGlobalTableActionsScope::AddDefaultHandlers(GetWidget());
}

WidgetsGlobalTableActionsScopeHandlersPtr WidgetTableViewWrapper::GetActionHandlers() const
{
    return WidgetsGlobalTableActionsScope::EditHandlers(GetWidget());
}

WidgetTableViewWrapper::WidgetTableViewWrapper(QTableView* tableView)
    : WidgetWrapper(tableView)
{}

void WidgetTableViewWrapper::DebugJson() const
{
    WidgetDialogWrapper dialog("DebugJson", []{
        DescCustomDialogParams result;
        result.SetView(new WidgetsDebugJsonDialog());
        result.SetResizeable();
        return result;
    });
    dialog.GetView<WidgetsDebugJsonDialog>()->SetTableView(GetWidget());
    dialog.Show();
}

void WidgetTableViewWrapper::DebugSelect() const
{
    auto* tableView = GetWidget();
    auto* viewModel = tableView->model();
    if(viewModel == nullptr) {
        return;
    }
    tableView->clearSelection();
    auto* selectionModel = tableView->selectionModel();
    qint32 countToSelect = 10;
    auto rowCount = viewModel->rowCount();
    if(rowCount == 0){
        return;
    }
    auto selectEach = rowCount / 10;
    qint32 currentRow = 0;
    for(qint32 i(0); i < countToSelect; ++i) {
        selectionModel->select(viewModel->index(currentRow % rowCount, 0), QItemSelectionModel::Select);
        currentRow += selectEach;
    }

    selectionModel->select(viewModel->index(rowCount - 1, 0), QItemSelectionModel::Select);
}

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
        text += "\t";
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
    if(includeHeaders) {
        text += tableView->model()->headerData(rowIndex, Qt::Vertical).toString() + "\t";
    }

    for(const auto& index : selectedIndexes) {
        if(index.data(LastEditRowRole).toBool() || header->isSectionHidden(index.column())) {
            continue;
        }
        if(rowIndex != index.row()) {
            rowIndex = index.row();
            if(!text.isEmpty()) {
                text.resize(text.size() - 1);
            }
            text += "\n";
            if(includeHeaders) {
                text += tableView->model()->headerData(rowIndex, Qt::Vertical).toString() + "\t";
            }
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
QCompleter* WidgetComboboxWrapper::CreateCompleter(QAbstractItemModel* model, const std::function<void (const QModelIndex& index)>& onActivated, qint32 column, QCompleter::ModelSorting sorting) const
{
    auto* combo = GetWidget();
    auto* oldEf = combo->property("a_completer_event_filter").value<EventFilterObject*>();
    if(oldEf != nullptr) {
        delete oldEf;
    }
    combo->setModel(model);
    combo->setEditable(true);
    auto* completer = new QCompleter(combo);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCompletionColumn(column);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(sorting);
    completer->setModel(model);
    if(onActivated != nullptr) {
        combo->connect(combo, QOverload<qint32>::of(&QComboBox::activated), [combo, onActivated](qint32 row){
            auto index = combo->model()->index(row, 0);
            combo->setCurrentText(index.data().toString());
            onActivated(index);
        });
    }
    combo->setModelColumn(column);
    combo->setCompleter(completer);

    auto* eventFilter = WidgetWrapper(combo).AddEventFilter([combo, completer](QObject*, QEvent* event) {
        if(event->type() == QEvent::KeyRelease && combo->currentText() == QString()) {
            completer->setCompletionPrefix(QString());
            completer->complete();
        }
        return false;
    });
    auto* eventFilterLineedit = WidgetWrapper(combo->lineEdit()).AddEventFilter([combo, completer](QObject*, QEvent* event) {
        if(event->type() == QEvent::MouseButtonRelease && combo->currentText() == QString()) {
            completer->setCompletionPrefix(QString());
            completer->complete();
        }
        return false;
    });
    eventFilterLineedit->setParent(eventFilter);
    combo->setProperty("a_completer_event_filter", QVariant::fromValue(eventFilter));

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

const WidgetComboboxWrapper& WidgetComboboxWrapper::AddViewModelEndEditHints(const std::function<void (QAbstractItemDelegate::EndEditHint)>& handler) const
{
    auto* comboBox = GetWidget();
    WidgetWrapper(comboBox->view()).AddEventFilter([handler, comboBox](QObject*, QEvent* e){
        if(e->type() == QEvent::KeyPress) {
            auto* ke = static_cast<QKeyEvent*>(e);
            switch(ke->key()) {
            case Qt::Key_Up:
                comboBox->setCurrentIndex(quint32(comboBox->currentIndex() - 1) % comboBox->count());
                return false;
            case Qt::Key_Down:
                comboBox->setCurrentIndex((comboBox->currentIndex() + 1) % comboBox->count());
                return false;
            case Qt::Key_Tab:
                handler(QAbstractItemDelegate::EditNextItem);
                return true;
            case Qt::Key_Backtab:
                handler(QAbstractItemDelegate::EditPreviousItem);
                return true;
            default: break;
            }
        }
        return false;
    });
    AddEventFilter([handler, comboBox](QObject*, QEvent* e){
        if(e->type() == QEvent::KeyPress) {
            auto* ke = static_cast<QKeyEvent*>(e);
            switch(ke->key()) {
            case Qt::Key_Up:
                comboBox->setCurrentIndex(quint32(comboBox->currentIndex() - 1) % comboBox->count());
                return false;
            case Qt::Key_Down:
                comboBox->setCurrentIndex((comboBox->currentIndex() + 1) % comboBox->count());
                return false;
            case Qt::Key_Backtab:
                handler(QAbstractItemDelegate::EditPreviousItem);
                return true;
            case Qt::Key_Tab:
                handler(QAbstractItemDelegate::EditNextItem);
                return true;
            default: break;
            }
        }
        return false;
    });
    return *this;
}

WidgetGroupboxWrapper::WidgetGroupboxWrapper(QGroupBox* groupBox)
    : WidgetWrapper(groupBox)
{}

LocalPropertyBool& WidgetGroupboxWrapper::WidgetChecked() const
{
    auto* widget = GetWidget();
    return *Injected<LocalPropertyBool>("a_checked", [&]() -> LocalPropertyBool* {
        auto* property = new LocalPropertyBool(widget->isChecked());
        property->ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget](bool value){
                                               widget->setChecked(value);
                                           });
        property->SetSetterHandler(ThreadHandlerMain);
        widget->connect(widget, &QGroupBox::toggled, [widget, property](qint32 state){
            *property = state;
        });
        return property;
    });
}

WidgetGroupboxLayoutWrapper::WidgetGroupboxLayoutWrapper(WidgetsGroupBoxLayout* groupBox)
    : WidgetWrapper(groupBox)
{

}

LocalPropertyBool& WidgetGroupboxLayoutWrapper::WidgetChecked() const
{
    return GetWidget()->Opened;
}

const WidgetGroupboxLayoutWrapper& WidgetGroupboxLayoutWrapper::AddCollapsing() const
{
    auto* widget = GetWidget();
    auto update = [widget](bool visible){
        auto animation = WidgetWrapper(widget).Injected<QPropertyAnimation>("a_collapsing_animation", [&]{
            return new QPropertyAnimation(widget, "maximumSize");
        });
        animation->stop();

        auto fullSize = QSize(widget->maximumWidth(), widget->sizeHint().height());
        auto minSize = QSize(widget->maximumWidth(), 44);
        animation->setDuration(200);
        animation->setStartValue(QSize(widget->maximumWidth(), widget->height()));
        animation->setEndValue(visible ? fullSize : minSize);
        animation->start();
    };
    widget->Opened.Connect(CDL, update);
    auto delayedUpdateCollapsing = DelayedCallObjectCreate(DelayedCallObjectParams());
    AddEventFilter([update, widget, delayedUpdateCollapsing](QObject*, QEvent* e) {
        switch(e->type()) {
        case QEvent::ShowToParent:
        case QEvent::StyleChange:
            delayedUpdateCollapsing->Call(CDL, [update, widget]{
                update(widget->Opened);
            });
            break;
        default:
            break;
        }

        return false;
    });
    return *this;
}

const WidgetGroupboxLayoutWrapper& WidgetGroupboxLayoutWrapper::AddCollapsingDispatcher(Dispatcher& updater, QScrollArea* area, qint32 delay) const
{
    auto collapsingData = InjectedCommutator("a_collapsing", [](QObject* w) {
                                 auto* widget = reinterpret_cast<WidgetsGroupBoxLayout*>(w);
                                 widget->Opened.Invoke();
                             }, 100);
    collapsingData->Commutator.ConnectFrom(CONNECTION_DEBUG_LOCATION, updater).MakeSafe(collapsingData->Connections);
    return *this;
}

WidgetComboboxWrapper::WidgetComboboxWrapper(QComboBox* combobox)
    : WidgetWrapper(combobox)
{}

WidgetComboboxWrapper::WidgetComboboxWrapper(WidgetsComboBoxLayout* combobox)
    : WidgetComboboxWrapper(combobox->comboBox())
{}

DisabledColumnComponentData& WidgetComboboxWrapper::disabledColumnComponent() const
{
    Q_ASSERT(qobject_cast<ViewModelsStandardListModel*>(GetWidget()->model()));
    auto* viewModel = reinterpret_cast<ViewModelsStandardListModel*>(GetWidget()->model());
    auto result = Injected<DisabledColumnComponentData>("a_disabledItemsComponentIndex", []{ return new DisabledColumnComponentData(); });
    if(!result->Installed) {
        ViewModelsTableColumnComponents::ColumnFlagsComponentData flagsComponent;
        flagsComponent.GetFlagsHandler = [result](qint32 row) -> std::optional<Qt::ItemFlags> {
            if(result->DisabledItems.contains(row)) {
                return ViewModelsTableBase::StandardNonEditableFlags() ^ Qt::ItemIsEnabled;
            }
            return std::nullopt;
        };
        viewModel->ColumnComponents.AddFlagsComponent(0, flagsComponent);
        result->Installed = true;
    }
    return *result;
}

const WidgetComboboxWrapper& WidgetComboboxWrapper::EnableStandardItems(const QSet<qint32>& indices) const
{
    auto* standardModel = qobject_cast<ViewModelsStandardListModel*>(GetWidget()->model());
    if(standardModel != nullptr) {
        disabledColumnComponent().DisabledItems -= indices;
    } else {
        auto* itemModel = qobject_cast<QStandardItemModel*>(GetWidget()->model());
        if(itemModel != nullptr){
            for(const auto& index : indices){
                auto* item = itemModel->item(index);
                if(item != nullptr) {
                    item->setFlags(item->flags().setFlag(Qt::ItemIsEnabled, true));
                }
            }
        } else {
            Q_ASSERT(false);
        }
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
    ApplyStyleProperty("w_highlighted", true);

    if(unhightlightIn > 0) {
        auto wrapper = *this;
        QTimer::singleShot(unhightlightIn, [wrapper]{
            wrapper.Lowlight();
        });
    }
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

void WidgetWrapper::UpdateStyle(bool recursive) const
{
    auto* target = GetWidget();
    auto updateStyle = [](QWidget* target) {
        auto* style = target->style();
        style->unpolish(target);
        style->polish(target);
        QEvent event(QEvent::StyleChange);
        QApplication::sendEvent(target, &event);
    };
    updateStyle(target);

    if(recursive) {
        auto childWidgets = target->findChildren<QWidget*>();
        for(auto* widget : childWidgets) {
            updateStyle(widget);
        }
    }
}

void WidgetWrapper::ApplyStyleProperty(const char* propertyName, const QVariant& value, bool recursive) const
{
    GetWidget()->setProperty(propertyName, value);
    UpdateStyle(recursive);
}

LocalPropertyBool& WidgetWrapper::WidgetEnablity() const
{
    auto value = m_object->property("a_enable").value<SharedPointer<LocalPropertyBool>>();
    if(value != nullptr) {
        return *value;
    }

    auto* spinbox = qobject_cast<QAbstractSpinBox*>(m_object);
    if(spinbox != nullptr) {
        return *Injected<LocalPropertyBool>("a_enable", [&]{
            auto* result = new LocalPropertyBool();
            auto& readOnly = *Injected<LocalPropertyBool>("a_readOnly", [&]() -> LocalPropertyBool* {
                auto* property = new LocalPropertyBool();
                property->EditSilent() = spinbox->isReadOnly();
                property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [spinbox, property]{
                    spinbox->setReadOnly(*property);
                    WidgetWrapper(spinbox).UpdateStyle();
                });
                property->SetSetterHandler(ThreadHandlerMain);
                return property;
            });
            readOnly.ConnectBoth(CDL, *result, FInverseBool, FInverseBool);
            return result;
        });
    }
    auto* lineEdit = qobject_cast<QLineEdit*>(m_object);
    if(lineEdit != nullptr) {
        return *Injected<LocalPropertyBool>("a_enable", [lineEdit]{
            auto* result = new LocalPropertyBool();
            WidgetLineEditWrapper(lineEdit).WidgetReadOnly().ConnectBoth(CDL, *result, FInverseBool, FInverseBool);
            return result;
        });
    }
    auto* textEdit = qobject_cast<QTextEdit*>(m_object);
    if(textEdit != nullptr) {
        return *Injected<LocalPropertyBool>("a_enable", [textEdit]{
            auto* result = new LocalPropertyBool();
            WidgetTextEditWrapper(textEdit).WidgetReadOnly().ConnectBoth(CDL, *result, FInverseBool, FInverseBool);
            return result;
        });
    }
    auto* widget = GetWidget();
    return *GetOrCreateProperty<LocalPropertyBool>("a_enable", [widget](QObject*, const LocalPropertyBool& visible){
        widget->setEnabled(visible);
    }, widget->isEnabled());
}

void WidgetWrapper::Lowlight() const
{
    ApplyStyleProperty("w_highlighted", false);
}

MainProgressBar* WidgetWrapper::AddModalProgressBar(const Name& processId) const
{
    return AddModalProgressBar(QSet<Name>({processId}));
}

MainProgressBar* WidgetWrapper::AddModalProgressBar(const QSet<Name>& processIds) const
{
    return AddModalProgressBar<MainProgressBar>(processIds);
}

const WidgetWrapper& WidgetWrapper::FixUp() const
{
    ForeachChildWidget([](const WidgetWrapper& w){
        auto* combobox = qobject_cast<QComboBox*>(w.GetWidget());
        if(combobox != nullptr) {
            WidgetWrapper(combobox->view()).ConnectFocus(combobox);
            combobox->style()->polish(combobox);
        }
    });

    ForeachChildWidget([](QWidget* sw){
        auto* w = qobject_cast<QAbstractItemView*>(sw);
        if(w == nullptr) {
            if(!qobject_cast<QAbstractSpinBox*>(sw) && !qobject_cast<QComboBox*>(sw)) {
                sw = nullptr;
            }
        } else {
            sw = w->viewport();
        }
        if(sw != nullptr) {
            WidgetWrapper(sw).AddEventFilter([](QObject* watched, QEvent* event) {
                if(event->type() == QEvent::Wheel) {
                    auto* widget = qobject_cast<QWidget*>(watched);
                    if(widget != nullptr && !widget->hasFocus()) {
                        event->ignore();
                        return true;
                    }
                }
                return false;
            });
            sw->setFocusPolicy(Qt::StrongFocus);
        }
    });

    return *this;
}

const WidgetComboboxWrapper& WidgetComboboxWrapper::DisableStandardItems(const QSet<qint32>& indices) const
{
    auto* standardModel = qobject_cast<ViewModelsStandardListModel*>(GetWidget()->model());
    if(standardModel != nullptr) {
        disabledColumnComponent().DisabledItems += indices;
    } else {
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
    }

    return *this;
}

#include <QDesktopWidget>

static void checkRestoredGeometry(const QRect &availableGeometry, QRect *restoredGeometry,
                                  int frameHeight)
{
    if (!restoredGeometry->intersects(availableGeometry)) {
        restoredGeometry->moveBottom(qMin(restoredGeometry->bottom(), availableGeometry.bottom()));
        restoredGeometry->moveLeft(qMax(restoredGeometry->left(), availableGeometry.left()));
        restoredGeometry->moveRight(qMin(restoredGeometry->right(), availableGeometry.right()));
    }
    restoredGeometry->moveTop(qMax(restoredGeometry->top(), availableGeometry.top() + frameHeight));
}

QByteArray WidgetWrapper::StoreGeometry() const
{
    auto* w = GetWidget();
    QDesktopWidget *desktop = QApplication::desktop();
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);
    const quint32 magicNumber = 0x1D9D0CB;
    // Version history:
    // - Qt 4.2 - 4.8.6, 5.0 - 5.3    : Version 1.0
    // - Qt 4.8.6 - today, 5.4 - today: Version 2.0, save screen width in addition to check for high DPI scaling.
    // - Qt 5.12 - today              : Version 3.0, save QWidget::geometry()
    quint16 majorVersion = 3;
    quint16 minorVersion = 0;
    const int screenNumber = desktop->screenNumber(w);
    stream << magicNumber
           << majorVersion
           << minorVersion
           << w->frameGeometry()
           << w->normalGeometry()
           << qint32(screenNumber)
           << quint8(w->windowState() & Qt::WindowMaximized)
           << quint8(w->windowState() & Qt::WindowFullScreen)
           << qint32(desktop->screenGeometry(screenNumber).width()) // added in 2.0
           << w->geometry(); // added in 3.0
    return array;
}

bool WidgetWrapper::RestoreGeometry(const QByteArray& geometry) const
{
    if (geometry.size() < 4)
            return false;
    QDataStream stream(geometry);
    stream.setVersion(QDataStream::Qt_4_0);
    QWidget* w = GetWidget();

    const quint32 magicNumber = 0x1D9D0CB;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if (storedMagicNumber != magicNumber)
        return false;

    const quint16 currentMajorVersion = 3;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;

    if (majorVersion > currentMajorVersion)
        return false;
    // (Allow all minor versions.)

    QRect restoredFrameGeometry;
    QRect restoredGeometry;
    QRect restoredNormalGeometry;
    qint32 restoredScreenNumber;
    quint8 maximized;
    quint8 fullScreen;
    qint32 restoredScreenWidth = 0;

    stream >> restoredFrameGeometry // Only used for sanity checks in version 0
           >> restoredNormalGeometry
           >> restoredScreenNumber
           >> maximized
           >> fullScreen;

    if (majorVersion > 1)
        stream >> restoredScreenWidth;
    if (majorVersion > 2)
        stream >> restoredGeometry;

    // ### Qt 6 - Perhaps it makes sense to dumb down the restoreGeometry() logic, see QTBUG-69104

    QDesktopWidget *desktop = QApplication::desktop();
    if (restoredScreenNumber >= desktop->numScreens())
        restoredScreenNumber = desktop->primaryScreen();
    const qreal screenWidthF = qreal(desktop->screenGeometry(restoredScreenNumber).width());
    // Sanity check bailing out when large variations of screen sizes occur due to
    // high DPI scaling or different levels of DPI awareness.
    if (restoredScreenWidth) {
        const qreal factor = qreal(restoredScreenWidth) / screenWidthF;
        if (factor < 0.8 || factor > 1.25)
            return false;
    } else {
        // Saved by Qt 5.3 and earlier, try to prevent too large windows
        // unless the size will be adapted by maximized or fullscreen.
        if (!maximized && !fullScreen && qreal(restoredFrameGeometry.width()) / screenWidthF > 1.5)
            return false;
    }

    const int frameHeight = 0;

    if (!restoredNormalGeometry.isValid())
        restoredNormalGeometry = QRect(QPoint(0, frameHeight), w->sizeHint());
    /*if (!restoredNormalGeometry.isValid()) {
        // use the widget's adjustedSize if the sizeHint() doesn't help
        restoredNormalGeometry.setSize(restoredNormalGeometry
                                       .size()
                                       .expandedTo(w->adjustedSize()));
    }*/

    const QRect availableGeometry = desktop->availableGeometry(restoredScreenNumber);

    checkRestoredGeometry(availableGeometry, &restoredGeometry, frameHeight);
    checkRestoredGeometry(availableGeometry, &restoredNormalGeometry, frameHeight);

    if (maximized || fullScreen) {
        // set geometry before setting the window state to make
        // sure the window is maximized to the right screen.
        Qt::WindowStates ws = w->windowState();

        if (ws & Qt::WindowFullScreen) {
            // Full screen is not a real window state on Windows.
            w->move(availableGeometry.topLeft());
        } else if (ws & Qt::WindowMaximized) {
            // Setting a geometry on an already maximized window causes this to be
            // restored into a broken, half-maximized state, non-resizable state (QTBUG-4397).
            // Move the window in normal state if needed.
            if (restoredScreenNumber != desktop->screenNumber(w)) {
                w->setWindowState(Qt::WindowNoState);
                w->setGeometry(restoredNormalGeometry);
            }
        } else {
            w->setGeometry(restoredNormalGeometry);
        }
        if (maximized)
            ws |= Qt::WindowMaximized;
        if (fullScreen)
            ws |= Qt::WindowFullScreen;
       w->setWindowState(ws);
       //normalGeometry = restoredNormalGeometry;
    } else {
        w->setWindowState(w->windowState() & ~(Qt::WindowMaximized | Qt::WindowFullScreen));
        if (majorVersion > 2)
            w->setGeometry(restoredGeometry);
        else
            w->setGeometry(restoredNormalGeometry);
    }
    return true;
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
        animation->setStartValue(QSize(widget->maximumWidth(), widget->height()));
        animation->setEndValue(visible ? fullSize : minSize);
        animation->start();
    };
    WidgetChecked().Connect(CDL, update);
    AddEventFilter([update, widget](QObject*, QEvent* e) {
        switch(e->type()) {
        case QEvent::ShowToParent:
        case QEvent::StyleChange:
            update(widget->isChecked());
            break;
        default:
            break;
        }

        return false;
    });
    return *this;
}

const WidgetGroupboxWrapper& WidgetGroupboxWrapper::AddCollapsingDispatcher(Dispatcher& updater, QScrollArea* area, qint32 delay) const
{
    auto collapsingData = InjectedCommutator("a_collapsing", [](QObject* w) {
                                 auto* widget = reinterpret_cast<QGroupBox*>(w);
                                 WidgetGroupboxWrapper(widget).WidgetChecked().Invoke();
                             }, 100);
    collapsingData->Commutator.ConnectFrom(CONNECTION_DEBUG_LOCATION, updater).MakeSafe(collapsingData->Connections);
    return *this;
}

WidgetWrapper::WidgetWrapper(QWidget* widget)
    : Super(widget)
{

}

const WidgetWrapper& WidgetWrapper::Click()
{
    QMouseEvent e(QEvent::MouseButtonPress, QPoint(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QMouseEvent er(QEvent::MouseButtonRelease, QPoint(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    qApp->sendEvent(m_object, &e);
    qApp->sendEvent(m_object, &er);
    return *this;
}

TranslatedStringPtr WidgetWrapper::WidgetToolTip() const
{
    auto* widget = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_tooltip", [widget](QObject*, const TranslatedString& text){
        widget->setToolTip(text);
    });
}


Dispatcher& WidgetWrapper::OnClicked() const
{
    auto w = *this;
    Q_ASSERT(!qobject_cast<QAbstractButton*>(w));
    return *Injected<Dispatcher>("a_on_clicked", [w]{
        auto* result = new Dispatcher();
        auto eventFilter = [result](QObject*, QEvent* event){
            switch (event->type()) {
            case QEvent::MouseButtonPress: result->Invoke(); break;
            default: break;
            }
            return false;
        };

        w.AddEventFilter(eventFilter);
        w.ForeachChildWidget([eventFilter](const WidgetWrapper& widget){
            widget.AddEventFilter(eventFilter);
        });

        WidgetsObserver::GetInstance().OnAdded.Connect(CONNECTION_DEBUG_LOCATION, [w, eventFilter](QObject* o){
            auto* created = qobject_cast<QWidget*>(o);
            if(WidgetWrapper(created).HasParent(w)) {
                WidgetWrapper(created).AddEventFilter(eventFilter);
            }
        }).MakeSafe(w.WidgetConnections());
        return result;
    });
}

const WidgetWrapper& WidgetWrapper::SetOnClicked(const FAction& action) const
{
    OnClicked().Connect(CONNECTION_DEBUG_LOCATION, action);
    return *this;
}

const WidgetWrapper& WidgetWrapper::SetToolTip(const FTranslationHandler& toolTip) const
{
    WidgetToolTip()->SetTranslationHandler(toolTip);
    return *this;
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

DispatcherConnections WidgetWrapper::ConnectVisibilityToInt(const char* debugLocation, const LocalPropertyInt& mode, const QVector<QWidget*>& widgets)
{
    qint32 i(0);
    DispatcherConnections result;
    for(auto* widget : widgets) {
        result += WidgetWrapper(widget).WidgetVisibility().ConnectFrom(debugLocation, [i](auto mode) {
            return mode == i;
        }, mode);
        ++i;
    }
    return result;
}

DispatcherConnections WidgetWrapper::CreateVisibilityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const
{
    auto result = createRule<WidgetWrapper>(debugLocation, QOverload<>::of(&WidgetWrapper::WidgetVisibility), handler, additionalWidgets, *dispatchers.first());
    for(auto* dispatcher : adapters::withoutFirst(dispatchers)) {
        result += WidgetVisibility().ConnectFromDispatchers(debugLocation, handler, *dispatcher);
    }
    return result;
}

DispatcherConnections WidgetWrapper::CreateEnablityRule(const char* debugLocation, const std::function<bool ()>& handler, const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets) const
{
    auto result = createRule<WidgetWrapper>(debugLocation, &WidgetWrapper::WidgetEnablity, handler, additionalWidgets, *dispatchers.first());
    for(auto* dispatcher : adapters::withoutFirst(dispatchers)) {
        result += WidgetEnablity().ConnectFromDispatchers(debugLocation, handler, *dispatcher);
    }
    return result;
}

void WidgetWrapper::ActivateWindow(int mode, qint32 delay) const
{
    ApplyStyleProperty("w_showfocus", mode);
    Q_ASSERT(delay > 0);
    auto wrapper = *this;
    auto valid = Injected<bool>("a_valid", [this]{
        auto* result = new bool(true);
        QObject::connect(GetWidget(), &QWidget::destroyed, [result]{ *result = false; });
        return result;
    });
    QTimer::singleShot(delay, [wrapper, valid]{
        if(!*valid) {
            return;
        }
        wrapper.ApplyStyleProperty("w_showfocus", 0);
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

WidgetLineEditWrapper::WidgetLineEditWrapper(class WidgetsLineEditLayout* lineEdit)
    : WidgetLineEditWrapper(lineEdit->lineEdit())
{

}

DispatcherConnectionsSafe& ObjectWrapper::ObjectConnections() const
{
    return *Injected<DispatcherConnectionsSafe>("a_connections");
}

EventFilterObject* ObjectWrapper::AddEventFilter(const std::function<bool (QObject*, QEvent*)>& filter) const
{
    return new EventFilterObject(filter, m_object);
}

const WidgetWrapper& WidgetWrapper::AddToFocusManager(const QVector<QWidget*>& additionalWidgets) const
{
    const auto& w = *this;
    auto eventFilter = [w](QObject* watched, QEvent* event){
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto toBlock = qobject_cast<QCheckBox*>(watched) || qobject_cast<QComboBox*>(watched);
            bool interupt = false;
            if(toBlock) {
                auto& fm = FocusManager::GetInstance();
                interupt = fm.FocusedWidget() != w;
            }
            FocusManager::GetInstance().SetFocusWidget(w);
            return interupt;
        }
        default: break;
        }
        return false;
    };

    w.AddEventFilter(eventFilter);
    w.ForeachChildWidget([eventFilter](const WidgetWrapper& widget){
        widget.AddEventFilter(eventFilter);
    });

    WidgetsObserver::GetInstance().OnAdded.Connect(CONNECTION_DEBUG_LOCATION, [w, eventFilter](QObject* o){
        auto* created = qobject_cast<QWidget*>(o);
        if(WidgetWrapper(created).HasParent(w)) {
            WidgetWrapper(created).AddEventFilter(eventFilter);
        }
    }).MakeSafe(w.WidgetConnections());


    QObject::connect(GetWidget(), &QWidget::destroyed, [w]{
        FocusManager::GetInstance().destroyed(w);
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

    auto& visible = WidgetVisibility();
    auto prevEffect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if(visible && fuzzyCompare(prevEffect->opacity(), opacity, 0.01)) {
        return;
    }

    auto prevOpacity = 0.0;
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

    auto& visible = WidgetVisibility();
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
    auto connection = animation->connect(animation.get(), &QPropertyAnimation::stateChanged, [widget](QAbstractAnimation::State newState, QAbstractAnimation::State){
        if(newState == QAbstractAnimation::Stopped) {
            WidgetWrapper(widget).WidgetVisibility() = false;
        }
    });
    widget->connect(widget, &QWidget::destroyed, [connection]{ QObject::disconnect(connection); });
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
    return ObjectConnections();
}

QVector<QWidget*>& WidgetWrapper::WidgetTrueFocusWidgets() const
{
    return *Injected<QVector<QWidget*>>("a_trueFocusWidgets");
}

LocalPropertySequentialEnum<HighLightEnum> & WidgetWrapper::WidgetHighlighted() const
{
    return *GetOrCreateProperty<LocalPropertySequentialEnum<HighLightEnum>>("a_highlighted", [](QObject* object, const LocalPropertySequentialEnum<HighLightEnum>& highlighted){
        WidgetWrapper(reinterpret_cast<QWidget*>(object)).ApplyStyleProperty("w_highlighted", highlighted.Value());
    }, HighLightEnum::None);
}

LocalPropertyBool& WidgetWrapper::WidgetVisibility() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_visible", [](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QWidget*>(object);
        action->setVisible(visible);
    }, GetWidget()->isVisible());
}

bool WidgetWrapper::HasParent(const QWidget* parent) const
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

LocalPropertyBool& ActionWrapper::WidgetChecked() const
{
    return *GetOrCreateProperty<QAction, LocalPropertyBool>("a_checked", [](QAction* action, const LocalPropertyBool& visible){
        action->setChecked(visible);
    }, [](QAction* btn, LocalPropertyBool* property){
        *property = btn->isChecked();
    }, &QAction::toggled, [](QAction* btn){
        btn->setCheckable(true);
    }, false);
}

LocalPropertyBool& MenuWrapper::WidgetVisibility() const
{
    return *GetOrCreateProperty<LocalPropertyBool>("a_visible", [](QObject* object, const LocalPropertyBool& visible){
        auto* action = reinterpret_cast<QMenu*>(object);
        action->menuAction()->setVisible(visible);
    }, true);
}

QMenu* MenuWrapper::GetMenu() const
{
    return GetWidget();
}

#ifdef UNITS_MODULE_LIB
ActionWrapper MenuWrapper::AddMeasurementAction(const Measurement* measurement, const QString& title, LocalPropertyDouble* value) const
{
    return AddDoubleAction(title, measurement->FromBaseToUnit(value->GetMin()), measurement->FromBaseToUnit(value->GetMax()), measurement->FromBaseToUnit(value->Native()), [value, measurement](const std::optional<double>& val){
        *value = measurement->FromUnitToBase(val.value_or(0.0));
    });
}
#endif

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

const MenuWrapper& MenuWrapper::SetIcon(const Name& iconName) const
{
    GetWidget()->setIcon(IconsManager::GetInstance().GetIcon(iconName));
    return *this;
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

const MenuWrapper& MenuWrapper::AddDebugActions() const
{
#ifndef BUILD_MASTER
    AddGlobalTableAction(GlobalActionDebugJSONId);
    AddGlobalTableAction(GlobalActionDebugSelectId);
#endif
    return *this;
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
        if(foundIt != m_globalActionsHandlers->Handlers.end()) {
            foundIt.value().SetVisible(true);
        }
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
        [](QPushButton* btn) { btn->setText(QObject::tr("Pick screen color")); },
        [](QPushButton* btn) { btn->setText(QObject::tr("Add to custom colors")); },
        [](QPushButton* btn) { delete btn; },
        [](QPushButton* btn) { delete btn; }
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

const WidgetColorDialogWrapper& WidgetColorDialogWrapper::SetShowAlpha(bool show) const
{
    GetWidget()->setOption(QColorDialog::ShowAlphaChannel, show);
    return *this;
}

ActionWrapper MenuWrapper::AddColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler) const
{
    static QPixmap pixmap(10,10);
    auto* btn = new QPushButton();
    pixmap.fill(color);
    btn->setIcon(pixmap);
    btn->setText(title);

    QObject::connect(btn, &QPushButton::clicked, [handler, color, btn] {
        auto result = WidgetsDialogsManager::GetInstance().GetColor(color, true);
        if(result.has_value()) {
            pixmap.fill(result.value());
            btn->setIcon(pixmap);
            handler(result.value());
        }
    });

    auto* action = new QWidgetAction(GetWidget());
    btn->setProperty(WidgetProperties::ActionWidget, true);
    action->setDefaultWidget(btn);
    GetWidget()->addAction(action);
    return action;
}

ActionWrapper MenuWrapper::AddDoubleAction(const QString& title, double min, double max, const std::optional<double>& value, const std::function<void (const std::optional<double>&)>& handler) const
{
    auto* widget = new QFrame();
    auto* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    widget->setLayout(layout);
    auto* label = new QLabel(title);
    auto* spinBox = new WidgetsDoubleSpinBoxWithCustomDisplay();
    spinBox->MakeOptional();
    layout->addWidget(label);
    layout->addWidget(spinBox);
    spinBox->SetValue(value);
    spinBox->setRange(min, max);
    QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [spinBox, handler](double value) {
        if(!spinBox->IsValid()) {
            handler(std::nullopt);
        } else {
            handler(value);
        }
    });
    auto* action = new QWidgetAction(GetWidget());
    widget->setProperty(WidgetProperties::ActionWidget, true);
    action->setDefaultWidget(widget);
    GetWidget()->addAction(action);
    return action;
}

ActionWrapper MenuWrapper::AddIntAction(const QString& title, qint32 value, const std::function<void (qint32)>& handler) const
{
    auto* widget = new QFrame();
    auto* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    widget->setLayout(layout);
    auto* label = new QLabel(title);
    auto* spinBox = new WidgetsSpinBoxWithCustomDisplay;
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    layout->addWidget(spinBox);
    spinBox->setValue(value);
    QObject::connect(spinBox, QOverload<qint32>::of(&WidgetsSpinBoxWithCustomDisplay::valueChanged), [handler](qint32 value) {
        handler(value);
    });
    auto* action = new QWidgetAction(GetWidget());
    widget->setProperty(WidgetProperties::ActionWidget, true);
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
        QWidgetAction* wa = qobject_cast<QWidgetAction*>(action);
        if(wa != nullptr) {
            return;
        }
        if (action && action->isEnabled()) {
            action->trigger();
        }
        else
            QMenu::mouseReleaseEvent(e);
    }
};

QMenu* MenuWrapper::CreatePreventedFromClosingMenu(const QString& title, QWidget* parent)
{
    return new PreventedFromClosingMenu(title, parent);
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
    return GetOrCreateProperty<TranslatedString>("a_text", [label](QObject*, const TranslatedString& text){
        label->setText(text.Native());
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

const WidgetWrapper& WidgetWrapper::AddTestHandler(const FAction& testHandler) const
{
#ifndef BUILD_MASTER///for test
    WidgetWrapper(GetWidget()).AddEventFilter([testHandler](QObject*, QEvent* event){
        if(event->type() == QEvent::KeyPress) {
            auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_T && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                testHandler();
                return true;
            }
        }
        return false;
    });
#endif
    return *this;
}

WidgetSpinBoxWrapper::WidgetSpinBoxWrapper(QSpinBox* widget)
    : Super(widget)
{}

WidgetSpinBoxWrapper::WidgetSpinBoxWrapper(WidgetsSpinBoxLayout* widget)
    : Super(widget->spinBox())
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

LocalPropertyBool& WidgetSpinBoxWrapper::WidgetReadOnly() const {
    return *Injected<LocalPropertyBool>("a_readOnly", [&]() -> LocalPropertyBool* {
        auto* property = new LocalPropertyBool();
        auto* widget = GetWidget();
        property->EditSilent() = widget->isReadOnly();
        property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [widget, property]{
            widget->setReadOnly(*property);
            WidgetWrapper(widget).UpdateStyle();
        });
        property->SetSetterHandler(ThreadHandlerMain);
        return property;
    });
}

WidgetDoubleSpinBoxWrapper::WidgetDoubleSpinBoxWrapper(QDoubleSpinBox* widget)
    : Super(widget)
{}

WidgetDoubleSpinBoxWrapper::WidgetDoubleSpinBoxWrapper(WidgetsDoubleSpinBoxLayout* widget)
    : Super(widget->spinBox())
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
            WidgetWrapper(widget).UpdateStyle();
        });
        property->SetSetterHandler(ThreadHandlerMain);
        return property;
    });
}

WidgetSplitterWrapper::WidgetSplitterWrapper(QSplitter* splitter)
    : Super(splitter)
{}

const WidgetSplitterWrapper& WidgetSplitterWrapper::SetInner(bool repolish) const
{
    if(repolish) {
        ApplyStyleProperty(WidgetProperties::InnerSplitter, true);
    } else {
        GetWidget()->setProperty(WidgetProperties::InnerSplitter, true);
    }
    return *this;
}

const WidgetSplitterWrapper& WidgetSplitterWrapper::SetWidgetSize(QWidget* widget, qint32 size) const
{
    auto index = GetWidget()->indexOf(widget);
    if(index != -1) {
        auto sizes = GetWidget()->sizes();
        auto reduceCount = sizes.size() - 1;
        auto delta = size - sizes[index];
        delta /= reduceCount;
        for(qint32 i(0); i < sizes.size(); ++i) {
            if(index == i) {
                sizes[i] = size;
            } else {
                sizes[i] -= delta;
            }
        }
        GetWidget()->setSizes(sizes);
    }
    return *this;
}

WidgetCheckBoxWrapper::WidgetCheckBoxWrapper(QCheckBox* target)
    : Super(target)
{}

LocalPropertyBool& WidgetCheckBoxWrapper::WidgetChecked() const
{
    auto* widget = GetWidget();
    return *Injected<LocalPropertyBool>("a_checked", [&]() -> LocalPropertyBool* {
        auto* property = new LocalPropertyBool(false);
        property->ConnectAndCall(CONNECTION_DEBUG_LOCATION, [widget](bool value){
                                               widget->setChecked(value);
                                           });
        property->SetSetterHandler(ThreadHandlerMain);
        widget->connect(widget, &QCheckBox::stateChanged, [widget, property](qint32 state){
            if(widget->property(WidgetProperties::ForceDisabled).toBool()) {
                widget->setCheckState(*property ? Qt::Checked : Qt::Unchecked);
                return;
            }
            *property = state;
            if(*property != (bool)state) {
                widget->setCheckState(*property ? Qt::Checked : Qt::Unchecked);
            }
        });
        return property;
    });
}

TranslatedStringPtr WidgetCheckBoxWrapper::WidgetText() const
{
    auto* label = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_text", [label](QObject*, const LocalPropertyString& text){
        label->setText(text);
    });
}

qint64 WidgetCheckBoxWrapper::GetAssignedFlag() const
{
    return GetWidget()->property("a_flag").toLongLong();
}

const WidgetCheckBoxWrapper& WidgetCheckBoxWrapper::AssignFlag(qint64 flag) const
{
    GetWidget()->setProperty("a_flag", flag);
    return *this;
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
    return Find(handler).row();
}

QModelIndex ViewModelWrapper::Find(const FIterationHandler& handler) const
{
    QModelIndex result;
    ForeachModelIndex([&](const QModelIndex& index) {
        if(handler(index)) {
            result = index;
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

WidgetTextEditWrapper::WidgetTextEditWrapper(QTextEdit* lineEdit)
    : Super(lineEdit)
{

}

const WidgetTextEditWrapper& WidgetTextEditWrapper::AddCompleter(const QStringList &keys) const
{
    auto* te = GetWidget();

    Injected<QCompleter>("a_completer", [keys, te]{
        auto completer = new QCompleter(keys);
        completer->setWidget(te);

        WidgetWrapper(completer->popup()).AddEventFilter([completer](QObject*, QEvent* e) {
            if(completer->popup()->currentIndex().row() != -1 && e->type() == QEvent::KeyPress) {
                auto ke = static_cast<QKeyEvent*>(e);
                switch(ke->key()) {
                case Qt::Key_Return:
                case Qt::Key_Enter:
                case Qt::Key_Tab:
                    emit completer->activated(completer->popup()->currentIndex().data().toString());
                    return true;
                default: break;
                }
            }
            return false;
        });

        auto start = ::make_shared<qint32>(0);
        auto end = ::make_shared<qint32>(0);

        completer->connect(completer, QOverload<const QString&>::of(&QCompleter::activated), [completer, te, start,end](const QString& text){
            if(*start == -1) {
                return;
            }
            auto trueStart = *start;
            te->setText(te->toPlainText().replace(trueStart, *end - trueStart, text));
            auto tc = te->textCursor();
            tc.setPosition(trueStart + text.length());
            te->setTextCursor(tc);
            completer->popup()->hide();
        });

        te->connect(te, &QTextEdit::cursorPositionChanged, [start,end, te, completer] {
            static QRegularExpression exp(R"((\$[\w\d\.]+))");

            auto newPos = te->textCursor().position();

            QString result;
            auto match = exp.globalMatch(te->toPlainText());
            while(match.hasNext()) {
                auto next = match.next();
                *start = next.capturedStart();
                *end = next.capturedEnd();
                if(newPos >= *start && newPos <= *end) {
                    completer->setCompletionPrefix(next.captured(1));
                    if(completer->popup()->model()->rowCount() != 1 || completer->popup()->model()->index(0,0).data().toString() != next.captured(1)) {
                        completer->complete();
                    }

                    return;
                }
            }
            *start = -1;
            *end = -1;
            completer->popup()->hide();
        });
        return completer;
    });

    return *this;
}

QString WidgetTextEditWrapper::Chopped(qint32 maxCount) const
{
    auto text = GetWidget()->toPlainText();
    auto withoutDots = maxCount - 3;
    if(text.size() > withoutDots) {
        return text.chopped(withoutDots) + "...";
    }
    return text;
}

LocalPropertyBool& WidgetTextEditWrapper::WidgetReadOnly() const
{
    return *Injected<LocalPropertyBool>("a_readOnly", [&]() -> LocalPropertyBool* {
        auto* property = new LocalPropertyBool();
        auto* widget = GetWidget();
        property->EditSilent() = widget->isReadOnly();
        property->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [widget, property]{
            widget->setReadOnly(*property);
            WidgetWrapper(widget).UpdateStyle();
        });
        property->SetSetterHandler(ThreadHandlerMain);
        return property;
    });
}


WidgetScrollAreaWrapper::WidgetScrollAreaWrapper(QScrollArea* button)
    : Super(button)
{

}

WidgetScrollAreaWrapper& WidgetScrollAreaWrapper::AddScrollByWheel(Qt::Orientation orientation)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_object->property("a_scroll_recursive_exists").toBool());
    m_object->setProperty("a_scroll_recursive_exists", true);
#endif

    auto recursionGuard = Injected<bool>("a_scroll_recursive", [orientation]{ return new bool(false); });
    auto* w = GetWidget();
    WidgetWrapper(w).AddEventFilter([w,recursionGuard, orientation](QObject*, QEvent* e){
        if(e->type() == QEvent::Wheel) {
            if(*recursionGuard) {
                return false;
            }
            guards::BooleanGuard guard(recursionGuard.get());
            if(orientation == Qt::Horizontal) {
                qApp->sendEvent(w->horizontalScrollBar(), e);
            } else {
                qApp->sendEvent(w->verticalScrollBar(), e);
            }
            return true;
        }
        return false;
    });

    return *this;
}

WidgetElidedLabelWrapper::WidgetElidedLabelWrapper(ElidedLabel* label)
    : WidgetWrapper(label)
{

}

TranslatedStringPtr WidgetElidedLabelWrapper::WidgetText() const
{
    auto* label = GetWidget();
    return GetOrCreateProperty<TranslatedString>("a_text", [label](QObject*, const TranslatedString& text){
        label->setText(text.Native());
    });
}

WidgetTabBarLayoutWrapper::WidgetTabBarLayoutWrapper(WidgetsTabBarLayout* groupBox)
    : WidgetWrapper(groupBox)
{

}

const WidgetTabBarLayoutWrapper& WidgetTabBarLayoutWrapper::AddCollapsing() const
{
    auto* widget = GetWidget();
    auto update = [widget](bool visible){
        auto animation = WidgetWrapper(widget).Injected<QPropertyAnimation>("a_collapsing_animation", [&]{
            return new QPropertyAnimation(widget, "maximumSize");
        });
        animation->stop();

        auto fullSize = QSize(widget->maximumWidth(), widget->sizeHint().height());
        auto minSize = QSize(widget->maximumWidth(), 40);
        animation->setDuration(200);
        animation->setStartValue(QSize(widget->maximumWidth(), widget->height()));
        animation->setEndValue(visible ? fullSize : minSize);
        animation->start();
    };
    widget->Opened.Connect(CDL, update);
    auto delayedUpdateCollapsing = DelayedCallObjectCreate(DelayedCallObjectParams());
    AddEventFilter([update, widget, delayedUpdateCollapsing](QObject*, QEvent* e) {
        switch(e->type()) {
        case QEvent::ShowToParent:
        case QEvent::StyleChange:
            delayedUpdateCollapsing->Call(CDL, [update, widget]{
                update(widget->Opened);
            });
            break;
        default:
            break;
        }

        return false;
    });
    return *this;
}

const WidgetTabBarLayoutWrapper& WidgetTabBarLayoutWrapper::AddCollapsingDispatcher(Dispatcher& updater, QScrollArea* area, qint32 delay) const
{
    auto collapsingData = InjectedCommutator("a_collapsing", [](QObject* w) {
                                 auto* widget = reinterpret_cast<WidgetsTabBarLayout*>(w);
                                 widget->Opened.Invoke();
                             }, 100);
    collapsingData->Commutator.ConnectFrom(CONNECTION_DEBUG_LOCATION, updater).MakeSafe(collapsingData->Connections);
    return *this;
}

WidgetProgressBarWrapper::WidgetProgressBarWrapper(QProgressBar* button)
    : Super(button)
{

}

WidgetProgressBarWrapper& WidgetProgressBarWrapper::SetId(const Name& processId)
{
    new ProgressBar(processId, GetWidget());
#ifdef QT_DEBUG
    Q_ASSERT(!GetWidget()->property("a_progressBar").toBool());
    GetWidget()->setProperty("a_progressBar", true);
#endif
    return *this;
}
