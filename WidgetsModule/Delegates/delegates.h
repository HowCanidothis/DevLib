#ifndef WIDGETS_DELEGATES_H
#define WIDGETS_DELEGATES_H

#include <QStyledItemDelegate>

#include <SharedModule/internal.hpp>

class DelegatesCombobox : public QStyledItemDelegate
{
    Q_OBJECT
public:
    DelegatesCombobox (Qt::AlignmentFlag aligment = Qt::AlignCenter, QObject* parent = nullptr);
    DelegatesCombobox (const QStringList& valuesList, Qt::AlignmentFlag aligment = Qt::AlignCenter, QObject* parent = nullptr);

    void setAligment(const Qt::AlignmentFlag& aligment);
    void setValues (const QStringList& valuesList);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    CommonDispatcher<class QComboBox*, const QModelIndex&> OnEditorAboutToBeShown;
private:
    QStringList m_values;
    Qt::AlignmentFlag m_aligment;
};

class DelegatesIntSpinBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesIntSpinBox (int min = std::numeric_limits<int>().lowest(), int max = (std::numeric_limits<int>().max)(), int step = 1, QObject* parent = nullptr);
    
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    
    void SetEditHandler(const std::function<bool(QAbstractItemModel*, const QModelIndex&)>& handler);
    void SetRange(int min, int max);
    
    CommonDispatcher<class QSpinBox*, const QModelIndex&> OnEditorAboutToBeShown;
    CommonDispatcher<int, const QModelIndex&> OnEditorValueChanged;
    CommonDispatcher<int, const QModelIndex&, bool&> OnEditingFinished;
    
private:
    int m_min;
    int m_max;
    int m_step;
    
    std::function<bool(QAbstractItemModel*, const QModelIndex&)> m_editHandler;
};

class DelegatesDoubleSpinBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesDoubleSpinBox (double min = std::numeric_limits<double>().lowest(), double max = (std::numeric_limits<double>().max)(), double step = 1.0, int precision = 2, QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    void SetEditHandler(const std::function<bool(QAbstractItemModel*, const QModelIndex&)>& handler);
    void SetRange(double min, double max);
    
    CommonDispatcher<class QDoubleSpinBox*, const QModelIndex&> OnEditorAboutToBeShown;
    CommonDispatcher<double, const QModelIndex&> OnEditorValueChanged;
    CommonDispatcher<double, const QModelIndex&, bool&> OnEditingFinished;
    
private:
    int m_precision;
    double m_min;
    double m_max;
    double m_step;
    
    std::function<bool(QAbstractItemModel*, const QModelIndex&)> m_editHandler;
    
};

//TODO ADD FORMAT
class DelegatesDateTime : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesDateTime (QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

signals:
    void dateTimeChanged(const QDateTime& dateTime, const QModelIndex& index);
};

class DelegatesCheckBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesCheckBox (QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
};
#endif // DELEGATES_H
