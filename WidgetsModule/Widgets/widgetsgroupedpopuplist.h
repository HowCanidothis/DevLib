#ifndef WIDGETSGROUPEDPOPUPLIST_H
#define WIDGETSGROUPEDPOPUPLIST_H

#include <QWidget>
#include <QVector>

#include <SharedModule/dispatcher.h>

#include "widgetspopuplistitem.h"

class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QScrollArea;

// Сгруппированный список для попапа: поиск по Title, группы по Group (порядок первого появления),
// на строке звезда / pin / ⋮, футер Create New. Состояние кнопок ведёт только модель.
// Наружу - диспетчеры, Qt-сигналов нет. Модель держит по SharedPointer (внешний владелец).
class WidgetsGroupedPopupList : public QWidget
{
    Q_OBJECT
    using Super = QWidget;

public:
    WidgetsGroupedPopupList(const WidgetsPopupListItemModelPtr& model, QWidget* parent = nullptr);
    ~WidgetsGroupedPopupList() override = default;

    // Футер Create New: скрыть, если создание здесь не предусмотрено (например, detached-область).
    void SetCreateNewVisible(bool visible);

    CommonDispatcher<const Name&> OnItemSelected;     // клик по Title, только для Enabled строк
    CommonDispatcher<const Name&> OnFavoriteToggled;  // клик по звезде (BtnFavorite)
    CommonDispatcher<const Name&> OnPinToggled;       // клик по BtnPin
    CommonDispatcher<const Name&> OnMoreRequested;    // клик по BtnMore
    CommonDispatcher<> OnCreateNewRequested;          // клик по BtnCreateNew

private:
    struct RowWidgets
    {
        WidgetsPopupListItem Item;  // снимок строки, по нему рисуется виджет
        qint32 ModelRow = -1;
        qint32 GroupIndex = -1;
        QFrame* Widget = nullptr;
        QPushButton* Title = nullptr;
        QPushButton* Favorite = nullptr;
        QPushButton* Pin = nullptr;
        QPushButton* More = nullptr;
    };

    struct GroupWidgets
    {
        QString Title;
        QLabel* Header = nullptr;   // nullptr для пустого Group: строки без заголовка
    };

    void onAboutToRemoveRows(qint32 first, qint32 last);
    void onRowsChanged(qint32 index, qint32 count);
    void rebuild();
    void rebuildFrom(const WidgetsPopupListItemContainer& items);
    void clearContent();
    RowWidgets createRow(const WidgetsPopupListItem& item, qint32 modelRow, qint32 groupIndex);
    void applyItem(RowWidgets& row, const WidgetsPopupListItem& item);
    void applyFilter();
    RowWidgets* findRowById(const Name& id);
    RowWidgets* findRowByModelRow(qint32 modelRow);

private:
    WidgetsPopupListItemModelPtr m_model;
    QLineEdit* m_search;
    QScrollArea* m_scroll;
    QWidget* m_content;
    QPushButton* m_createNew;
    QVector<RowWidgets> m_rows;     // в порядке отображения (по группам)
    QVector<GroupWidgets> m_groups;
    DispatcherConnectionsSafe m_connections; // последним: лямбды ссылаются на члены выше
};

#endif // WIDGETSGROUPEDPOPUPLIST_H
