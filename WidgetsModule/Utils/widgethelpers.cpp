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

#include <optional>

WidgetsLocalPropertyColorWrapper::WidgetsLocalPropertyColorWrapper(QWidget* widget, const Stack<WidgetsLocalPropertyColorWrapperColorMap>& colorMap)
    : m_widget(widget)
{
    widget->installEventFilter(this);
    connect(widget, &QWidget::destroyed, [this]{
        delete this;
    });

    m_properties.Swap(const_cast<Stack<WidgetsLocalPropertyColorWrapperColorMap>&>(colorMap));

    for(auto& propertyMap : m_properties) {
        propertyMap.Color->OnChange.Connect(this, [this]{
            m_updateLater.Call([this]{
                polish();
            });
        }).MakeSafe(m_connections);
    }
}

bool WidgetsLocalPropertyColorWrapper::eventFilter(QObject*, QEvent* e)
{
    if(e->type() == QEvent::StyleChange) {
        m_updateLater.Call([this]{
            polish();
        });
    }
    return false;
}

WidgetsLocalPropertyVisibilityWrapper::WidgetsLocalPropertyVisibilityWrapper(QWidget* widget)
    : Visible(true)
{
    connect(widget, &QWidget::destroyed, [this]{
        delete this;
    });
    Visible.OnChange.Connect(this, [this, widget]{
        widget->setVisible(Visible);
    }).MakeSafe(m_connections);
    widget->setVisible(Visible);
}

WidgetsLocalPropertyEnablityWrapper::WidgetsLocalPropertyEnablityWrapper(QWidget* widget)
    : Enabled(true)
{
    connect(widget, &QWidget::destroyed, [this]{
        delete this;
    });
    Enabled.OnChange.Connect(this, [this, widget]{
        widget->setEnabled(Enabled);
    }).MakeSafe(m_connections);
    widget->setEnabled(Enabled);
}


void WidgetsLocalPropertyColorWrapper::polish()
{
    auto pal = m_widget->palette();
    for(const auto& propertyMap : m_properties) {
        pal.setColor((QPalette::ColorRole)propertyMap.Role, *propertyMap.Color);
    }
    m_widget->setPalette(pal);
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

bool WidgetsObserver::eventFilter(QObject *watched, QEvent *e)
{
    if(e->type() == QEvent::ChildAdded) {
        OnAdded(watched);
    }
    return false;
}

DispatcherConnection WidgetAppearance::ConnectWidgetsByVisibility(WidgetsLocalPropertyVisibilityWrapper* base, WidgetsLocalPropertyVisibilityWrapper* child)
{
    return child->Visible.ConnectFrom(base->Visible);
}

void WidgetAppearance::SetVisibleAnimated(QWidget* widget, bool visible)
{
    if(visible) {
        WidgetAppearance::ShowAnimated(widget);
    } else {
        WidgetAppearance::HideAnimated(widget);
    }
}

static const char* WidgetAppearanceAnimationPropertyName = "WidgetAppearanceAnimation";

Q_DECLARE_METATYPE(SharedPointer<QPropertyAnimation>)

void WidgetAppearance::ShowAnimated(QWidget* widget)
{
    auto prevAnimation = widget->property(WidgetAppearanceAnimationPropertyName).value<SharedPointer<QPropertyAnimation>>();
    if(prevAnimation != nullptr) {
        prevAnimation->stop();
    }

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    SharedPointer<QPropertyAnimation> animation(new QPropertyAnimation(effect,"opacity"));
    widget->setProperty(WidgetAppearanceAnimationPropertyName, QVariant::fromValue(animation));
    animation->setDuration(2000);
    animation->setStartValue(0.0);
    animation->setEndValue(0.8);
    animation->setEasingCurve(QEasingCurve::InBack);
    animation->start();
    widget->show();
}

void WidgetAppearance::HideAnimated(QWidget* widget)
{
    auto prevAnimation = widget->property(WidgetAppearanceAnimationPropertyName).value<SharedPointer<QPropertyAnimation>>();
    if(prevAnimation != nullptr) {
        prevAnimation->stop();
    }

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    SharedPointer<QPropertyAnimation> animation(new QPropertyAnimation(effect,"opacity"));
    widget->setProperty(WidgetAppearanceAnimationPropertyName, QVariant::fromValue(animation));
    animation->setDuration(2000);
    animation->setStartValue(0.8);
    animation->setEndValue(0);
    animation->setEasingCurve(QEasingCurve::OutBack);
    animation->connect(animation.get(), &QPropertyAnimation::finished, [widget]{
        widget->hide();
    });
    animation->start();
}

void WidgetContent::ForeachChildWidget(QWidget* target, const std::function<void (QWidget*)>& handler)
{
    auto childWidgets = target->findChildren<QWidget*>();
    for(auto* childWidget : childWidgets) {
        handler(childWidget);
    }
}

QList<int> WidgetContent::SelectedRowsSorted(QTableView* tableView)
{
    auto ret = SelectedRowsSet(tableView).toList();
    std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QList<int> WidgetContent::SelectedColumnsSorted(QTableView* tableView)
{
	auto ret = SelectedColumnsSet(tableView).toList();
	std::sort(ret.begin(), ret.end(),[](const int& v1, const int& v2){ return v1 < v2; });
    return ret;
}

QSet<int> WidgetContent::SelectedRowsSet(QTableView* tableView)
{
	QSet<int> set;
    auto selectedIndexes = tableView->selectionModel()->selectedIndexes();

    for(const auto& index : selectedIndexes){
        set.insert(index.row());
    }
    return set;
}

QSet<int> WidgetContent::SelectedColumnsSet(QTableView* tableView)
{
    QSet<int> set;
    auto selectedIndexes = tableView->selectionModel()->selectedIndexes();

    for(const auto& index : selectedIndexes){
        set.insert(index.column());
    }
    return set;
}

bool WidgetContent::HasParent(QWidget* child, QWidget* parent)
{
    bool result = false;
    ForeachParentWidget(child, [&result, parent](QWidget* gypoParent){
        if(gypoParent == parent) {
            result = true;
            return true;
        }
        return false;
    });
    return result;
}

void WidgetContent::ForeachParentWidget(QWidget* target, const std::function<bool(QWidget*)>& handler)
{
    auto* parent = target->parentWidget();
    while(parent != nullptr) {
        if(handler(parent)) {
            break;
        }
        parent = parent->parentWidget();
    }
}

void WidgetContent::SelectRowsAndScrollToFirst(QTableView* table, const QSet<qint32>& rowIndices)
{
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

void WidgetContent::CopySelectedTableContentsToClipboard(QTableView* tableView)
{
    auto selectedIndexes = tableView->selectionModel()->selectedIndexes();
    if(selectedIndexes.isEmpty()) {
        return;
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
}

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

QLineEdit* WidgetsAttachment::AttachLineEditAdjuster(QLineEdit* edit) {
    auto invalidate = [edit]{
        QFontMetrics fm(edit->font());
        int pixelsWide = fm.width(edit->text());
        pixelsWide += edit->contentsMargins().left() + edit->contentsMargins().right() + 20;
        edit->setMinimumWidth(pixelsWide);
    };
    QObject::connect(edit, &QLineEdit::textChanged, invalidate);
    invalidate();
    return edit;
}

bool WidgetsAttachment::eventFilter(QObject* watched, QEvent* e)
{
    return m_filter(watched, e);
}

void WidgetsAttachment::AttachBlockEnter(QObject* target)
{
    auto blockEnter = [](QObject*, QEvent* event) {
        if(event->type() == QEvent::KeyRelease || event->type() == QEvent::KeyPress) {
            auto* keyEvent = reinterpret_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Return) {
                return true;
            }
        }
        return false;
    };
    WidgetsAttachment::Attach(target, blockEnter);
}

void WidgetContent::ComboboxDisconnectModel(QComboBox* combo)
{
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
}
