#include "widgetsgroupedpopuplist.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>

namespace {

// Динамическое свойство для QSS ([disabled="true"]) + перепримение стиля.
void setStyleProperty(QWidget* widget, const char* name, bool value)
{
    const QVariant current = widget->property(name);
    if(current.isValid() && current.toBool() == value) {
        return;
    }
    widget->setProperty(name, value);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

// Иконки и размеры - из QSS темы по objectName.
QPushButton* createRowButton(const char* objectName, bool checkable)
{
    auto* button = new QPushButton();
    button->setObjectName(objectName);
    button->setFlat(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setCheckable(checkable);
    return button;
}

} // namespace

WidgetsGroupedPopupList::WidgetsGroupedPopupList(const WidgetsPopupListItemModelPtr& model, QWidget* parent)
    : Super(parent)
    , m_model(model)
    , m_search(new QLineEdit())
    , m_scroll(new QScrollArea())
    , m_content(nullptr)
    , m_createNew(new QPushButton())
{
    Q_ASSERT(m_model != nullptr);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_search->setObjectName("SearchEdit");
    m_search->setPlaceholderText(tr("Search"));
    m_search->setClearButtonEnabled(true);
    layout->addWidget(m_search);

    m_scroll->setObjectName("PopupListScroll");
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_scroll, 1);

    m_createNew->setObjectName("BtnCreateNew");
    m_createNew->setText(tr("Create New"));
    m_createNew->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(m_createNew);

    connect(m_search, &QLineEdit::textChanged, this, [this]{
        applyFilter();
    });
    connect(m_createNew, &QPushButton::clicked, this, [this]{
        OnCreateNewRequested();
    });

    // Вставка может поменять порядок групп (первое появление) - полная перестройка.
    m_model->OnRowsInserted.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32, qint32){
        rebuild();
    }).MakeSafe(m_connections);

    // Таблица не вызывает OnAboutToRemoveRows (удаление идёт через OnReset); подписка на случай, если появится.
    m_model->OnAboutToRemoveRows.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32 first, qint32 count){
        onAboutToRemoveRows(first, first + count - 1);
    }).MakeSafe(m_connections);

    // Второй аргумент - count, не last (см. TModelsTableWrapper::Edit).
    m_model->OnRowsChanged.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32 index, qint32 count, const QSet<qint32>&){
        onRowsChanged(index, count);
    }).MakeSafe(m_connections);

    m_model->OnReset.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        rebuild();
    }).MakeSafe(m_connections);

    rebuild();
}

void WidgetsGroupedPopupList::onAboutToRemoveRows(qint32 first, qint32 last)
{
    // Строим по снимку без удаляемых строк.
    WidgetsPopupListItemContainer rest;
    const qint32 size = m_model->GetSize();
    for(qint32 row = 0; row < size; ++row) {
        if(row < first || row > last) {
            rest.append(m_model->At(row));
        }
    }
    rebuildFrom(rest);
}

void WidgetsGroupedPopupList::onRowsChanged(qint32 index, qint32 count)
{
    for(qint32 row = index; row < index + count; ++row) {
        if(!m_model->IsValidRow(row)) {
            rebuild();
            return;
        }
        const WidgetsPopupListItem& item = m_model->At(row);
        RowWidgets* rowWidgets = findRowByModelRow(row);
        if(rowWidgets == nullptr || rowWidgets->Item.Id != item.Id || rowWidgets->Item.Group != item.Group) {
            // Сменился Id или Group: точечно не сверить, порядок групп мог измениться.
            rebuild();
            return;
        }
        applyItem(*rowWidgets, item);
    }
    applyFilter(); // Title мог измениться
}

void WidgetsGroupedPopupList::rebuild()
{
    WidgetsPopupListItemContainer items;
    const qint32 size = m_model->GetSize();
    items.reserve(size);
    for(qint32 row = 0; row < size; ++row) {
        items.append(m_model->At(row));
    }
    rebuildFrom(items);
}

void WidgetsGroupedPopupList::rebuildFrom(const WidgetsPopupListItemContainer& items)
{
    clearContent();

    m_content = new QWidget();
    m_content->setObjectName("PopupListContent");
    auto* layout = new QVBoxLayout(m_content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Группы в порядке первого появления, строки внутри группы - в порядке модели.
    QVector<QString> groupOrder;
    QVector<QVector<qint32>> groupRows;
    for(qint32 row = 0; row < items.size(); ++row) {
        qint32 groupIndex = groupOrder.indexOf(items.at(row).Group);
        if(groupIndex == -1) {
            groupIndex = groupOrder.size();
            groupOrder.append(items.at(row).Group);
            groupRows.append(QVector<qint32>());
        }
        groupRows[groupIndex].append(row);
    }

    for(qint32 groupIndex = 0; groupIndex < groupOrder.size(); ++groupIndex) {
        GroupWidgets group;
        group.Title = groupOrder.at(groupIndex);
        if(!group.Title.isEmpty()) {
            group.Header = new QLabel(group.Title);
            group.Header->setObjectName("GroupHeader");
            layout->addWidget(group.Header);
        }
        m_groups.append(group);

        for(qint32 row : groupRows.at(groupIndex)) {
            m_rows.append(createRow(items.at(row), row, groupIndex));
            layout->addWidget(m_rows.last().Widget);
        }
    }
    layout->addStretch(1);

    m_scroll->setWidget(m_content);
    applyFilter();
}

void WidgetsGroupedPopupList::clearContent()
{
    m_rows.clear();
    m_groups.clear();
    if(m_content == nullptr) {
        return;
    }
    QWidget* old = m_scroll->takeWidget();
    Q_ASSERT(old == m_content);
    m_content = nullptr;
    if(old == nullptr) {
        return;
    }
    // Перестройка может прийти из обработчика клика (подписчик правит модель): удаляем отложенно.
    for(auto* button : old->findChildren<QPushButton*>()) {
        disconnect(button, nullptr, this, nullptr);
    }
    old->hide();
    old->deleteLater();
}

WidgetsGroupedPopupList::RowWidgets WidgetsGroupedPopupList::createRow(const WidgetsPopupListItem& item, qint32 modelRow, qint32 groupIndex)
{
    RowWidgets row;
    row.ModelRow = modelRow;
    row.GroupIndex = groupIndex;

    row.Widget = new QFrame();
    row.Widget->setObjectName("PopupListRow");
    auto* layout = new QHBoxLayout(row.Widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    row.Title = createRowButton("BtnTitle", false);
    row.Title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    row.Favorite = createRowButton("BtnFavorite", true);  // иконка из QSS (:checked)
    row.Pin = createRowButton("BtnPin", true);            // иконка из QSS (:checked)
    row.More = createRowButton("BtnMore", false);
    row.More->setText(QString(QChar(0x22EE)));            // ⋮

    layout->addWidget(row.Title, 1);
    layout->addWidget(row.Favorite);
    layout->addWidget(row.Pin);
    layout->addWidget(row.More);

    const Name id = item.Id;

    connect(row.Title, &QPushButton::clicked, this, [this, id]{
        RowWidgets* current = findRowById(id);
        if(current == nullptr || !current->Item.Enabled) {
            return; // Enabled=false: выбор строки запрещён
        }
        OnItemSelected(id);
    });

    // Checkable-кнопка переключается сама до clicked: возвращаем состояние модели, наружу - только событие.
    connect(row.Favorite, &QPushButton::clicked, this, [this, id]{
        if(RowWidgets* current = findRowById(id)) {
            current->Favorite->setChecked(current->Item.Favorite);
        }
        OnFavoriteToggled(id);
    });

    connect(row.Pin, &QPushButton::clicked, this, [this, id]{
        if(RowWidgets* current = findRowById(id)) {
            current->Pin->setChecked(current->Item.Pinned);
        }
        OnPinToggled(id);
    });

    connect(row.More, &QPushButton::clicked, this, [this, id]{
        OnMoreRequested(id);
    });

    applyItem(row, item);
    return row;
}

void WidgetsGroupedPopupList::applyItem(RowWidgets& row, const WidgetsPopupListItem& item)
{
    row.Item = item;
    row.Title->setText(item.Title);
    row.Title->setToolTip(item.Title);
    row.Favorite->setChecked(item.Favorite);
    row.Pin->setChecked(item.Pinned);

    // Серый текст - через QSS по свойству; звезда / pin / ⋮ остаются активными.
    const bool disabled = !item.Enabled;
    setStyleProperty(row.Widget, "disabled", disabled);
    setStyleProperty(row.Title, "disabled", disabled);
}

void WidgetsGroupedPopupList::applyFilter()
{
    const QString filter = m_search->text().trimmed();
    QVector<bool> groupVisible(m_groups.size(), false);

    for(const RowWidgets& row : m_rows) {
        const bool visible = filter.isEmpty() || row.Item.Title.contains(filter, Qt::CaseInsensitive);
        row.Widget->setVisible(visible);
        if(visible) {
            groupVisible[row.GroupIndex] = true;
        }
    }

    for(qint32 groupIndex = 0; groupIndex < m_groups.size(); ++groupIndex) {
        QLabel* header = m_groups.at(groupIndex).Header;
        if(header != nullptr) {
            header->setVisible(groupVisible.at(groupIndex));
        }
    }
}

WidgetsGroupedPopupList::RowWidgets* WidgetsGroupedPopupList::findRowById(const Name& id)
{
    for(RowWidgets& row : m_rows) {
        if(row.Item.Id == id) {
            return &row;
        }
    }
    return nullptr;
}

WidgetsGroupedPopupList::RowWidgets* WidgetsGroupedPopupList::findRowByModelRow(qint32 modelRow)
{
    for(RowWidgets& row : m_rows) {
        if(row.ModelRow == modelRow) {
            return &row;
        }
    }
    return nullptr;
}

void WidgetsGroupedPopupList::SetCreateNewVisible(bool visible)
{
    m_createNew->setVisible(visible);
}
