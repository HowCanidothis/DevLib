#ifndef WIDGETSREACTIVESTRIPBUTTONS_H
#define WIDGETSREACTIVESTRIPBUTTONS_H

#include <functional>

#include <QHash>
#include <QObject>
#include <QPointer>

#include <PropertiesModule/Models/wrappers.h>
#include <SharedModule/dispatcher.h>

#include "../Layouts/horizontalscrollstrip.h"

// Строка данных полосы. Контейнер хранит только такие строки, кнопки живут отдельно.
struct WidgetsStripItem
{
    Name Id;
    QString Text;
    QString ToolTip;
    bool Enabled = true;

    bool operator==(const WidgetsStripItem& other) const
    {
        return Id == other.Id && Text == other.Text && ToolTip == other.ToolTip && Enabled == other.Enabled;
    }
    bool operator!=(const WidgetsStripItem& other) const { return !(*this == other); }
};
DECLARE_MODEL_BY_TYPE(WidgetsStripItem)

// Реактивно синхронизирует кнопки HorizontalScrollStrip со строками WidgetsStripItemModel по Id.
// Снаружи чёрный ящик: модель и полоса на входе, диспетчер ButtonClicked(Id) на выходе.
// Модель держит по SharedPointer (внешний владелец), полоса - родитель обёртки или живёт дольше.
class WidgetsReactiveStripButtons : public QObject
{
    using Super = QObject;

public:
    // Клик по кнопке строки: Id строки.
    CommonDispatcher<const Name&> ButtonClicked;
    // Id нажатой (checked) кнопки; остальные отжаты. Клик сам состояние не меняет - его задаёт владелец через CheckedId.
    LocalPropertyName CheckedId;

    using FButtonFactory = std::function<QPushButton* (const WidgetsStripItem&)>;

    // factory == nullptr -> new QPushButton (checkable) с Text / ToolTip / Enabled из строки.
    WidgetsReactiveStripButtons(HorizontalScrollStrip* strip, const WidgetsStripItemModelPtr& model,
                         const FButtonFactory& factory = nullptr, QObject* parent = nullptr);
    ~WidgetsReactiveStripButtons() override = default;

private:
    QPushButton* getButton(const Name& id);
    const WidgetsStripItem& itemAt(qint32 row) const;
    void onRowsInserted(qint32 first, qint32 count);
    void onAboutToRemoveRows(qint32 first, qint32 count);
    void onRowsChanged(qint32 first, qint32 count);
    void rebuild();
    void updateChecked();

    void createButton(const WidgetsStripItem& item, qint32 index);
    void removeButton(const Name& id);
    void detachButton(const QPointer<QPushButton>& button);
    void removeAllButtons();

private:
    QPointer<HorizontalScrollStrip> m_strip;
    FButtonFactory m_factory;
    WidgetsStripItemModelPtr m_model;
    QHash<Name, QPointer<QPushButton>> m_buttons;
    DispatcherConnectionsSafe m_connections; // последним: лямбды ссылаются на члены выше
};

#endif // WIDGETSREACTIVESTRIPBUTTONS_H
