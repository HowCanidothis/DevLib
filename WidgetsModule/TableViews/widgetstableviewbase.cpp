#include "widgetstableviewbase.h"

#include <QHeaderView>
#include <QKeyEvent>

#include "WidgetsModule/widgetsdeclarations.h"
#include "WidgetsModule/Utils/widgethelpers.h"

IMPLEMENT_GLOBAL_CHAR_1(OverridenEditorEvent)
using EditorEvent = std::function<void (const FAction&)>;
using WidgetsTableViewBaseOverridenEditorEvent = SharedPointer<EditorEvent>;
Q_DECLARE_METATYPE(WidgetsTableViewBaseOverridenEditorEvent)

void WidgetsTableViewBase::OverrideEditorEvent(const std::function<void (const FAction&)>& action)
{
    setProperty(OverridenEditorEvent, QVariant::fromValue(::make_shared<EditorEvent>(action)));
}

void WidgetsTableViewBase::commitData(QWidget* editor)
{
    m_editorIndex = currentIndex();
    Super::commitData(editor);
}

void WidgetsTableViewBase::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
    qint32 crow;
    QHeaderView* header;
    qint32 vcolumn;
    std::function<QModelIndex (qint32 row, qint32 column)> indexExtractor;
    std::function<qint32 (const QModelIndex&)> columnExtractor;
    QModelIndex index = m_editorIndex;
    qint32 rcount;
    qint32 ccount;

    FAction findNext;
    FAction findNextRow;
    QModelIndex nextToSelect;
    qint32 increment;
    if(hint == QAbstractItemDelegate::EditNextItem) {
        increment = 1;
        findNext = [&]{
            for(qint32 i(vcolumn); i < ccount; ++i) {
                auto cIndex = indexExtractor(crow, i);
                if(!header->isSectionHidden(columnExtractor(cIndex)) && (cIndex.flags() & Qt::ItemIsEditable)) {
                    nextToSelect = cIndex;
                    break;
                }
            }
        };
        findNextRow = [&]{
            vcolumn = 0;
            auto c = rcount - crow;
            for(qint32 i(0); i < c; ++i) {
                ++crow;
                findNext();
                if(nextToSelect.isValid()) {
                    break;
                }
            }
        };
    } else if(hint == QAbstractItemDelegate::EditPreviousItem) {
        increment = -1;
        findNext = [&]{
            for(qint32 i(vcolumn); i >= 0; --i) {
                auto cIndex = indexExtractor(crow, i);
                if(!header->isSectionHidden(columnExtractor(cIndex)) && (cIndex.flags() & Qt::ItemIsEditable)) {
                    nextToSelect = cIndex;
                    break;
                }
            }
        };
        findNextRow = [&]{
            vcolumn = ccount - 1;
            auto c = crow;
            for(qint32 i(0); i < c; ++i) {
                --crow;
                findNext();
                if(nextToSelect.isValid()) {
                    break;
                }
            }
        };
    }

    if(findNext == nullptr) {
        Super::closeEditor(editor, hint);
        return;
    }

    if(model()->property(WidgetProperties::InvertedModel).toBool()) {
        crow = index.column();
        header = verticalHeader();
        vcolumn = header->visualIndex(index.row()) + increment;
        indexExtractor = [this, header](qint32 crow, qint32 i) {
            return model()->index(header->logicalIndex(i), crow);
        };
        columnExtractor = [](const QModelIndex& i) {
            return i.row();
        };
        ccount = model()->rowCount();
        rcount = model()->columnCount();
    } else {
        crow = index.row();
        header = horizontalHeader();
        vcolumn = header->visualIndex(index.column()) + increment;
        indexExtractor = [this, header](qint32 crow, qint32 i) {
            return model()->index(crow, header->logicalIndex(i));
        };
        columnExtractor = [](const QModelIndex& i) {
            return i.column();
        };
        ccount = model()->columnCount();
        rcount = model()->rowCount();
    }
    Super::closeEditor(editor, QAbstractItemDelegate::NoHint);

    findNext();

    if(!nextToSelect.isValid()) {
        findNextRow();
    }

    if(!nextToSelect.isValid()) {
        return;
    }

    auto editItem = [this](const QModelIndex& newRowModelIndex){
        auto faction = [this, newRowModelIndex]{
            auto indexFlags = newRowModelIndex.flags();
            selectionModel()->setCurrentIndex(newRowModelIndex, QItemSelectionModel::SelectCurrent);

            // currentChanged signal would have already started editing
            if (indexFlags & Qt::ItemIsEditable
                && (!(editTriggers() & QAbstractItemView::CurrentChanged))) {
                edit(newRowModelIndex);
            }
        };
        auto e = property(OverridenEditorEvent);
        if(e.isValid()) {
            (*e.value<WidgetsTableViewBaseOverridenEditorEvent>())(faction);
        } else {
            faction();
        }
    };

    editItem(nextToSelect);
}

void WidgetsTableViewBase::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if(state() != EditingState) {
            edit(currentIndex());
        }
    }
    Super::keyPressEvent(event);
}

