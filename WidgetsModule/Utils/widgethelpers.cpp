#include "widgethelpers.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTableView>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QKeyEvent>
#include <QAction>
#include <QGroupBox>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QMenu>

#include <optional>

#include <ActionsModule/internal.hpp>

#include "WidgetsModule/Managers/widgetsfocusmanager.h"
#include "WidgetsModule/Managers/widgetsstandardtableheadermanager.h"

#include "WidgetsModule/ProgressBars/mainprogressbar.h"

#include "WidgetsModule/TableViews/Header/widgetsresizableheaderattachment.h"

#include "WidgetsModule/Attachments/widgetsactivetableattachment.h"
#include "WidgetsModule/Attachments/tableviewwidgetattachment.h"

Q_DECLARE_METATYPE(SharedPointer<DelayedCallObject>)

class WidgetsAttachment : public QObject
{
    using Super = QObject;
public:
    using FFilter = std::function<bool (QObject*, QEvent*)>;
    WidgetsAttachment(const FFilter& filter, QObject* parent);

    static void Attach(QObject* target, const FFilter& filter);

private:
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
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

WidgetLineEditWrapper& WidgetLineEditWrapper::SetDynamicSizeAdjusting() {
    auto* edit = lineEdit();
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

bool WidgetsAttachment::eventFilter(QObject* watched, QEvent* e)
{
    return m_filter(watched, e);
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

        callDelayed->Call([this, o]{
            OnAdded(reinterpret_cast<QWidget*>(o));
        });
        o->setProperty("a_command", QVariant::fromValue(callDelayed));
    }
    return false;
}

static const char* WidgetAppearanceAnimationPropertyName = "WidgetAppearanceAnimation";

Q_DECLARE_METATYPE(SharedPointer<QPropertyAnimation>)

WidgetsMatchingAttachment* WidgetTableViewWrapper::CreateMatching(QAbstractItemModel* targetModel, const QSet<qint32>& targetImportColumns)
{
    return new WidgetsMatchingAttachment(tableView(), targetModel, targetImportColumns);
}

QHeaderView* WidgetTableViewWrapper::InitializeHorizontal(const DescColumnsParams& params)
{
    auto* tableView = this->tableView();
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(Qt::Horizontal, tableView);
    tableView->setHorizontalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto* editScope = ActionsManager::GetInstance().FindScope("Edit");
    if(editScope != nullptr){
        auto actions = editScope->GetActionsQList();
        tableView->addActions(actions);
    }
    tableView->addAction(dragDropHeader->CreateShowColumsMenu(nullptr, params)->menuAction());

    tableView->setWordWrap(true);
    auto* verticalHeader = tableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    WidgetsActiveTableViewAttachment::Attach(tableView);
    WidgetsStandardTableHeaderManager::GetInstance().Register(params.StateTag, dragDropHeader);
    return dragDropHeader;
}

QHeaderView* WidgetTableViewWrapper::InitializeVertical(const DescColumnsParams& params)
{
    auto* tableView = this->tableView();
    auto* dragDropHeader = new WidgetsResizableHeaderAttachment(Qt::Vertical, tableView);
    tableView->setVerticalHeader(dragDropHeader);
    tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto* editScope = ActionsManager::GetInstance().FindScope("Edit");
    if(editScope != nullptr){
        auto actions = editScope->GetActionsQList();
        tableView->addActions(actions);
    }
    tableView->addAction(dragDropHeader->CreateShowColumsMenu(nullptr, params)->menuAction());

    tableView->setWordWrap(true);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    WidgetsActiveTableViewAttachment::Attach(tableView);
    WidgetsStandardTableHeaderManager::GetInstance().Register(params.StateTag, dragDropHeader);
    return dragDropHeader;
}

QList<int> WidgetTableViewWrapper::SelectedRowsSorted()
{
    auto ret = SelectedRowsSet().toList();
    std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QList<int> WidgetTableViewWrapper::SelectedColumnsSorted()
{
    auto ret = SelectedColumnsSet().toList();
	std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QSet<int> WidgetTableViewWrapper::SelectedRowsSet()
{
	QSet<int> set;
    auto selectedIndexes = tableView()->selectionModel()->selectedIndexes();

    for(const auto& index : selectedIndexes){
        set.insert(index.row());
    }
    return set;
}

QSet<int> WidgetTableViewWrapper::SelectedColumnsSet()
{
    QSet<int> set;
    auto selectedIndexes = tableView()->selectionModel()->selectedIndexes();

    for(const auto& index : selectedIndexes){
        set.insert(index.column());
    }
    return set;
}

void WidgetTableViewWrapper::SelectRowsAndScrollToFirst(const QSet<qint32>& rowIndices)
{
    auto* table = tableView();
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

void WidgetTableViewWrapper::SelectColumnsAndScrollToFirst(const QSet<qint32>& columnIndices)
{
    auto* table = tableView();
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

bool WidgetTableViewWrapper::CopySelectedTableContentsToClipboard()
{
    auto* tableView = this->tableView();
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

WidgetComboboxWrapper& WidgetComboboxWrapper::DisconnectModel()
{
    auto* combo = this->combobox();
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

WidgetComboboxWrapper& WidgetComboboxWrapper::EnableStandardItems(const QSet<qint32>& indices)
{
    auto* itemModel = qobject_cast<QStandardItemModel*>(combobox()->model());
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

WidgetWrapper& WidgetWrapper::AddModalProgressBar()
{
    Q_ASSERT(m_widget->isWindow());
    new MainProgressBar(m_widget, Qt::Window | Qt::FramelessWindowHint);
    auto* widget = m_widget;
    AddEventFilter([widget](QObject*, QEvent* event){
        if(event->type() == QEvent::Close) {
            event->ignore();
            widget->close();
            return true;
        }
        return false;
    });
#ifdef QT_DEBUG
    Q_ASSERT(!m_widget->property("a_progressBar").toBool());
    m_widget->setProperty("a_progressBar", true);
#endif
    return *this;
}

WidgetWrapper& WidgetWrapper::FixUp()
{
    ForeachChildWidget([](QWidget* w){
        auto* combobox = qobject_cast<QComboBox*>(w);
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

WidgetComboboxWrapper& WidgetComboboxWrapper::DisableStandardItems(const QSet<qint32>& indices)
{
    auto* itemModel = qobject_cast<QStandardItemModel*>(combobox()->model());
    if(itemModel != nullptr){
        for(const auto& index : indices){
            auto* item = itemModel->item(index);
            item->setFlags(item->flags().setFlag(Qt::ItemIsEnabled, false));
        }
    } else {
        Q_ASSERT(false);
    }
    return *this;
}

WidgetGroupboxWrapper& WidgetGroupboxWrapper::AddCollapsing()
{
    auto* widget = this->groupBox();
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
    ThreadsBase::DoMain([widget, update]{
        update(widget->isChecked());
    });
    return *this;
}

WidgetGroupboxWrapper& WidgetGroupboxWrapper::AddCollapsingDispatcher(Dispatcher* updater)
{
    auto collapsingData = InjectedCommutator("a_collapsing", [](QWidget* w) {
                                 auto* widget = reinterpret_cast<QGroupBox*>(w);
                                 if(widget->isChecked()) {
                                     ThreadsBase::DoMain([widget]{
                                         widget->setMaximumSize(QSize(widget->maximumWidth(), widget->sizeHint().height()));
                                     });
                                 }
                             });
    collapsingData->Commutator.Subscribe(CONNECTION_DEBUG_LOCATION, updater).MakeSafe(collapsingData->Connections);
    return *this;
}

WidgetWrapper::WidgetWrapper(QWidget* widget)
    : m_widget(widget)
{

}

DispatcherConnection WidgetWrapper::ConnectVisibility(const char* debugLocation, QWidget* another)
{
    return WidgetWrapper(another).WidgetVisibility().ConnectFrom(debugLocation, WidgetVisibility());
}

DispatcherConnection WidgetWrapper::ConnectEnablity(const char* debugLocation, QWidget* another)
{
    return WidgetWrapper(another).WidgetEnablity().ConnectFrom(debugLocation, WidgetEnablity());
}

void WidgetWrapper::SetVisibleAnimated(bool visible, int duration, double opacity)
{
    if(visible) {
        ShowAnimated(duration, opacity);
    } else {
        HideAnimated(duration);
    }
}

WidgetWrapper& WidgetWrapper::BlockWheel()
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

WidgetWrapper& WidgetWrapper::AddEventFilter(const std::function<bool (QObject*, QEvent*)>& filter)
{
    WidgetsAttachment::Attach(m_widget, filter);
    return *this;
}

WidgetWrapper& WidgetWrapper::AddToFocusManager(const QVector<QWidget*>& additionalWidgets)
{
    auto* target = m_widget;
    auto eventFilter = [target](QObject*, QEvent* event){
        switch (event->type()) {
        case QEvent::FocusIn: FocusManager::GetInstance().SetFocusWidget(target); break;
        case QEvent::Destroy: FocusManager::GetInstance().destroyed(target);
            break;
        default: break;
        }
        return false;
    };

    auto childEventFilter = [target](QObject* , QEvent* event){
        switch (event->type()) {
        case QEvent::FocusIn: FocusManager::GetInstance().SetFocusWidget(target); break;
        default: break;
        }
        return false;
    };

    AddEventFilter(eventFilter);
    ForeachChildWidget([childEventFilter](QWidget* widget){
        WidgetWrapper(widget).AddEventFilter(childEventFilter);
    });
    for(auto* child : additionalWidgets) {
        WidgetWrapper(child).AddEventFilter(childEventFilter);
    }

    return *this;
}

void WidgetWrapper::ShowAnimated(int duration, double opacity)
{
    auto* widget = m_widget;
    auto prevAnimation = widget->property(WidgetAppearanceAnimationPropertyName).value<SharedPointer<QPropertyAnimation>>();
    if(prevAnimation != nullptr) {
        prevAnimation->stop();
    }

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    SharedPointer<QPropertyAnimation> animation(new QPropertyAnimation(effect,"opacity"));
    widget->setProperty(WidgetAppearanceAnimationPropertyName, QVariant::fromValue(animation));
    animation->setDuration(duration);
    animation->setStartValue(0.0);
    animation->setEndValue(opacity);
    animation->setEasingCurve(QEasingCurve::InBack);
    animation->start();
    widget->show();
}

void WidgetWrapper::HideAnimated(int duration)
{
    auto* widget = m_widget;
    auto prevAnimation = widget->property(WidgetAppearanceAnimationPropertyName).value<SharedPointer<QPropertyAnimation>>();
    if(prevAnimation != nullptr) {
        prevAnimation->stop();
    }

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    SharedPointer<QPropertyAnimation> animation(new QPropertyAnimation(effect,"opacity"));
    widget->setProperty(WidgetAppearanceAnimationPropertyName, QVariant::fromValue(animation));
    animation->setDuration(duration);
    animation->setStartValue(widget->windowOpacity());
    animation->setEndValue(0);
    animation->setEasingCurve(QEasingCurve::OutBack);
    animation->start();
    animation->connect(animation.get(), &QPropertyAnimation::finished, [widget]{
        widget->hide();
    });
    widget->show();
}

WidgetWrapper& WidgetWrapper::SetPalette(const QHash<qint32, LocalPropertyColor*>& palette)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_widget->property("a_palette").toBool());
#endif
    auto connections = DispatcherConnectionsSafeCreate();
    auto updater = DelayedCallObjectCreate();
    auto* pWidget = m_widget;
    auto update = updater->Wrap([pWidget, palette]{
        auto pal = pWidget->palette();
        for(auto it(palette.cbegin()), e(palette.cend()); it != e; ++it) {
            pal.setColor((QPalette::ColorRole)it.key(), *it.value());
        }
        pWidget->setPalette(pal);
    });

    AttachEventFilter([updater, update, connections](QObject*, QEvent* e){
        if(e->type() == QEvent::StyleChange) {
            update();
        }
        return false;
    });

    for(const auto* color : palette) {
        color->OnChanged.Connect(this, [update]{
            update();
        }).MakeSafe(*connections);
    }

    m_widget->setProperty("a_palette", true);
    return *this;
}

DispatcherConnections WidgetWrapper::createRule(const char* debugLocation, LocalPropertyBool* property, const std::function<bool ()>& handler,
                                                const QVector<Dispatcher*>& dispatchers, const QVector<QWidget*>& additionalWidgets,
                                                const FConnector& connector)
{
    DispatcherConnections result;
    result += property->ConnectFrom(debugLocation, [handler] { return handler(); }, dispatchers);
    for(auto* widget : additionalWidgets) {
        result += (this->*connector)(debugLocation, widget);
    }
    return result;
}

LocalPropertyBool& WidgetWrapper::WidgetVisibility(bool animated)
{
    return *getOrCreateProperty<LocalPropertyBool>("a_visible", [animated](QWidget* action, const LocalPropertyBool& visible){
        if(animated) {
            WidgetWrapper(action).SetVisibleAnimated(visible);
        } else {
            action->setVisible(visible);
        }
    }, true);
}

LocalPropertyBool& WidgetWrapper::WidgetEnablity()
{
    return *getOrCreateProperty<LocalPropertyBool>("a_enable", [](QWidget* action, const LocalPropertyBool& visible){
        action->setEnabled(visible);
    }, true);
}

bool WidgetWrapper::HasParent(QWidget* parent)
{
    bool result = false;
    ForeachParentWidget([&result, parent](QWidget* gypoParent){
        if(gypoParent == parent) {
            result = true;
            return true;
        }
        return false;
    });
    return result;
}

void WidgetWrapper::ForeachParentWidget(const std::function<bool(QWidget*)>& handler)
{
    auto* parent = m_widget->parentWidget();
    while(parent != nullptr) {
        if(handler(parent)) {
            break;
        }
        parent = parent->parentWidget();
    }
}

void WidgetWrapper::ForeachChildWidget(const std::function<void (QWidget*)>& handler)
{
    auto childWidgets = m_widget->findChildren<QWidget*>();
    for(auto* childWidget : childWidgets) {
        handler(childWidget);
    }
}

WidgetWrapper& WidgetWrapper::AttachEventFilter(const std::function<bool (QObject*, QEvent*)>& eventFilter)
{
    WidgetsAttachment::Attach(m_widget, eventFilter);
    return *this;
}
