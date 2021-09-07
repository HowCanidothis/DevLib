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
protected:
    QStringList m_values;
    Qt::AlignmentFlag m_aligment;
};

class DelegatesIntSpinBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesIntSpinBox (QObject* parent = nullptr);
    
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    
    void SetEditHandler(const std::function<bool(QAbstractItemModel*, const QModelIndex&)>& handler);
    
    CommonDispatcher<class QSpinBox*, const QModelIndex&> OnEditorAboutToBeShown;
    CommonDispatcher<int, const QModelIndex&> OnEditorValueChanged;
    CommonDispatcher<int, const QModelIndex&, bool&> OnEditingFinished;
    
private:    
    std::function<bool(QAbstractItemModel*, const QModelIndex&)> m_editHandler;
};

class DelegatesDoubleSpinBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesDoubleSpinBox (QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    void SetEditHandler(const std::function<bool(QAbstractItemModel*, const QModelIndex&)>& handler);
    
    CommonDispatcher<class QDoubleSpinBox*, const QModelIndex&> OnEditorAboutToBeShown;
    CommonDispatcher<double, const QModelIndex&> OnEditorValueChanged;
    CommonDispatcher<double, const QModelIndex&, bool&> OnEditingFinished;
    
protected:
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

	void SetLocale(const QLocale& locale);
	void SetDisplayFormat(const QString& format) { m_displayFormat = format; }
	QString displayText(const QVariant& value, const QLocale& locale) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	
	CommonDispatcher<class QWidget*, const QModelIndex&> OnEditorAboutToBeShown;
	CommonDispatcher<QDateTime, const QModelIndex&> OnEditorValueChanged;
protected:
	QString m_displayFormat;
	QLocale m_locale;
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
