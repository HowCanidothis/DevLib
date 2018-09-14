#ifndef PROPERTIESSTYLEDDELEGATELISTENER_H
#define PROPERTIESSTYLEDDELEGATELISTENER_H

#include <QObject>
#include <QModelIndex>
class QStyledItemDelegate;

class PropertiesStyledDelegateListener : public QObject
{
    Q_OBJECT
    QWidget* _editor;
    QModelIndex _modelIndex;
    const QStyledItemDelegate* _delegate;
public:
    PropertiesStyledDelegateListener(QWidget* editor, const QModelIndex& modelIndex, const QStyledItemDelegate* delegate);

public Q_SLOTS:
    void onEditorValueChanged();
};
#endif // ONEDITORVALUECHANGEDLISTENER_H
