#ifndef PROPERTIESDELEGATE_H
#define PROPERTIESDELEGATE_H

#ifdef QT_GUI_LIB

#include <QStyledItemDelegate>

class PropertiesDelegate : public QStyledItemDelegate
{
    typedef QStyledItemDelegate Super;
public:
    PropertiesDelegate(QObject* parent);

    // QAbstractItemDelegate interface
public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    QString displayText(const QVariant& value, const QLocale& locale) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    friend class PropertiesViewBase;
    QColor m_gradientLeft;
    QColor m_gradientRight;
    double m_gradientRightBorder;
};

#endif

#endif // PROPERTIESVIEWDELEGATE_H
