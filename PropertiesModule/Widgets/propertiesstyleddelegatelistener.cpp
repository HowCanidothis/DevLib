#include "propertiesstyleddelegatelistener.h"

#ifdef QT_GUI_LIB

#include <QStyledItemDelegate>

PropertiesStyledDelegateListener::PropertiesStyledDelegateListener(QWidget* editor, const QModelIndex& modelIndex, const QStyledItemDelegate* delegate)
    : QObject(editor)
    , _editor(editor)
    , _modelIndex(modelIndex)
    , _delegate(delegate)
{}

void PropertiesStyledDelegateListener::onEditorValueChanged() {
    _delegate->setModelData(_editor, const_cast<QAbstractItemModel*>(_modelIndex.model()), _modelIndex);
}

#endif
