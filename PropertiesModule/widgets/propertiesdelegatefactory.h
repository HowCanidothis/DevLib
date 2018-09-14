#ifndef PROPERTIESDELEGATEFACTORY_H
#define PROPERTIESDELEGATEFACTORY_H

#include <QString>

class QWidget;
class QStyleOptionViewItem;
class QModelIndex;
class QAbstractItemModel;
class QStyledItemDelegate;
class QLocale;
class QVariant;

class PropertiesDelegateFactory
{
public:
    static void SetFactory(PropertiesDelegateFactory* factory);
    static const PropertiesDelegateFactory& Instance();

    virtual QWidget* CreateEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const;
    virtual bool SetModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    virtual bool SetEditorData(QWidget* editor, const QModelIndex& index, const QStyledItemDelegate* delegate) const;
    virtual bool DisplayText(QString& ret, const QVariant& value, const QLocale& locale) const;

protected:
    PropertiesDelegateFactory();

private:
    static PropertiesDelegateFactory*& currentFactory();
};

#endif // PROPERTIESDELEGATEWIDGETSFACTORY_H
