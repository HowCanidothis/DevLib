#include "widgetsreactivestripbuttons.h"

#include <QPushButton>

#include "../Utils/widgethelpers.h"

namespace {

void applyItem(QPushButton* button, const WidgetsStripItem& item)
{
    button->setText(item.Text);
    button->setToolTip(item.ToolTip);
    button->setEnabled(item.Enabled);
}

} // namespace

WidgetsReactiveStripButtons::WidgetsReactiveStripButtons(HorizontalScrollStrip* strip, const WidgetsStripItemModelPtr& model,
                                           const FButtonFactory& factory, QObject* parent)
    : Super(parent)
    , m_strip(strip)
    , m_factory(factory)
    , m_model(model)
{
    Q_ASSERT(strip != nullptr);
    Q_ASSERT(m_model != nullptr);

    if(m_factory == nullptr) {
        m_factory = [](const WidgetsStripItem&) {
            auto* button = new QPushButton();
            button->setCheckable(true);
            return button;
        };
    }

    m_model->OnRowsInserted.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32 first, qint32 count){
        onRowsInserted(first, count);
    }).MakeSafe(m_connections);

    m_model->OnAboutToRemoveRows.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32 first, qint32 count){
        onAboutToRemoveRows(first, count); // строки ещё в контейнере, Id можно прочитать
    }).MakeSafe(m_connections);

    m_model->OnRowsChanged.Connect(CONNECTION_DEBUG_LOCATION, [this](qint32 first, qint32 count, const QSet<qint32>&){
        onRowsChanged(first, count);
    }).MakeSafe(m_connections);

    m_model->OnReset.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        rebuild();
    }).MakeSafe(m_connections);

    CheckedId.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        updateChecked();
    }).MakeSafe(m_connections);

    rebuild();
}

QPushButton* WidgetsReactiveStripButtons::getButton(const Name& id)
{
    auto it = m_buttons.find(id);
    if(it == m_buttons.end()) {
        return nullptr;
    }
    if(it.value().isNull()) {
        m_buttons.erase(it); // кнопку удалили извне
        return nullptr;
    }
    return it.value().data();
}

const WidgetsStripItem& WidgetsReactiveStripButtons::itemAt(qint32 row) const
{
    Q_ASSERT(m_model->IsValidRow(row));
    return m_model->At(row);
}

void WidgetsReactiveStripButtons::onRowsInserted(qint32 first, qint32 count)
{
    if(m_strip.isNull()) {
        return;
    }
    for(qint32 row = first; row < first + count; ++row) {
        createButton(itemAt(row), row);
    }
}

void WidgetsReactiveStripButtons::onAboutToRemoveRows(qint32 first, qint32 count)
{
    for(qint32 row = first; row < first + count; ++row) {
        removeButton(itemAt(row).Id);
    }
}

void WidgetsReactiveStripButtons::onRowsChanged(qint32 first, qint32 count)
{
    for(qint32 row = first; row < first + count; ++row) {
        const WidgetsStripItem& item = itemAt(row);
        QPushButton* button = getButton(item.Id);
        if(button == nullptr) {
            // Id сменился или кнопку удалили извне: точечно не сверить.
            rebuild();
            return;
        }
        applyItem(button, item);
    }
}

void WidgetsReactiveStripButtons::rebuild()
{
    removeAllButtons();
    if(m_strip.isNull()) {
        return;
    }
    const qint32 size = m_model->GetSize();
    for(qint32 row = 0; row < size; ++row) {
        createButton(itemAt(row), row);
    }
}

void WidgetsReactiveStripButtons::updateChecked()
{
    for(auto it = m_buttons.cbegin(); it != m_buttons.cend(); ++it) {
        if(!it.value().isNull()) {
            it.value()->setChecked(it.key() == CheckedId.Native());
        }
    }
}

void WidgetsReactiveStripButtons::createButton(const WidgetsStripItem& item, qint32 index)
{
    removeButton(item.Id); // дубликат Id заменяет старую кнопку

    QPushButton* button = m_factory(item);
    if(button == nullptr) {
        return; // фабрика вернула nullptr: строка без кнопки
    }

    applyItem(button, item);
    button->setChecked(item.Id == CheckedId.Native());
    // Контекст - this: Qt снимет соединение при уничтожении обёртки.
    WidgetPushButtonWrapper(button).SetOnClicked([this, id = item.Id]{
        ButtonClicked(id);
        // checkable-кнопка переключилась кликом: возвращаем состояние по CheckedId (обработчик мог удалить кнопку).
        QPushButton* clicked = getButton(id);
        if(clicked != nullptr) {
            clicked->setChecked(id == CheckedId.Native());
        }
    });

    // Порядок кнопок совпадает с порядком строк модели; индекс не больше числа созданных кнопок.
    const qint32 position = qMin(index, static_cast<qint32>(m_buttons.size()));
    m_strip->InsertWidget(position, button);
    m_buttons.insert(item.Id, button);
}

void WidgetsReactiveStripButtons::removeButton(const Name& id)
{
    auto it = m_buttons.find(id);
    if(it == m_buttons.end()) {
        return;
    }
    const QPointer<QPushButton> button = it.value();
    m_buttons.erase(it);
    detachButton(button);
}

void WidgetsReactiveStripButtons::detachButton(const QPointer<QPushButton>& button)
{
    if(button.isNull()) {
        return;
    }
    disconnect(button.data(), nullptr, this, nullptr);
    if(!m_strip.isNull()) {
        m_strip->RemoveWidget(button.data());
    }
    button->deleteLater();
}

void WidgetsReactiveStripButtons::removeAllButtons()
{
    const auto buttons = m_buttons;
    m_buttons.clear();
    for(const auto& button : buttons) {
        detachButton(button);
    }
}
