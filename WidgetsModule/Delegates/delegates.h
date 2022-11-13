#ifndef WIDGETS_DELEGATES_H
#define WIDGETS_DELEGATES_H

#include <QStyledItemDelegate>

#include <SharedModule/internal.hpp>

class DelegatesCombobox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesCombobox (QObject* parent);
    DelegatesCombobox (const std::function<QStringList ()>& valuesExtractor, QObject* parent);

    DelegatesCombobox* SetAlignment(Qt::AlignmentFlag alignment) { m_aligment = alignment; return this; }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    CommonDispatcher<class QComboBox*, const QModelIndex&> OnEditorAboutToBeShown;
    CommonDispatcher<class QComboBox*, const QModelIndex&> OnAboutToSetModelData;
protected:
    std::function<QStringList ()> m_valuesExtractor;
    Qt::AlignmentFlag m_aligment;
};

class DelegatesComboboxCustomViewModel : public DelegatesCombobox
{
    using Super = DelegatesCombobox;
public:
    using ModelGetter = std::function<QAbstractItemModel* ()>;
    DelegatesComboboxCustomViewModel(const ModelGetter& getter, QObject* parent);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex&) const override;

protected:
    ModelGetter m_getter;
};

///todo make base
class DelegatesComboboxSelector : public DelegatesComboboxCustomViewModel
{
    using Super = DelegatesComboboxCustomViewModel;
public:
    using ComparatorHandler = std::function<bool(const QModelIndex& TableData, const QModelIndex& EditorData)>;
    using ValueExtractorHandler = std::function<QVariant(const QModelIndex& EditorData)>;

    DelegatesComboboxSelector(const ModelGetter& getter, QObject* parent);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& tableIndex) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    DelegatesComboboxSelector& SetCustomComparator(const ComparatorHandler& handler){
        m_comparator = handler;
        return *this;
    }
    DelegatesComboboxSelector& SetValueExtractor(const ValueExtractorHandler& handler){
        m_extractor = handler;
        return *this;
    }

private:
    ComparatorHandler m_comparator;
    ValueExtractorHandler m_extractor;

    static ComparatorHandler IdComparator;
    static ValueExtractorHandler IdExtractor;
};

class DelegatesIntSpinBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesIntSpinBox (QObject* parent);
    
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
    DelegatesDoubleSpinBox (QObject* parent);

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
    std::function<bool(QAbstractItemModel*, const QModelIndex&)> m_editHandler;
};

//TODO ADD FORMAT
class DelegatesDateTime : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesDateTime (QObject* parent);

	QString displayText(const QVariant& value, const QLocale& locale) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	
	CommonDispatcher<class QWidget*, const QModelIndex&> OnEditorAboutToBeShown;
	CommonDispatcher<QDateTime, const QModelIndex&> OnEditorValueChanged;
};

class DelegatesCheckBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesCheckBox (QObject* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
};

class DelegatesDateTimePicker : public DelegatesDateTime
{
    using Super = DelegatesDateTime;
public:
    DelegatesDateTimePicker(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {}
    void setModelData(QWidget* , QAbstractItemModel* model, const QModelIndex& index) const override;
};

struct DateTimeRangeAttachment {
    static void Attach(DelegatesDateTimePicker* delegate, const QPair<int, int>& columns, LocalPropertyDateTime* start = nullptr, LocalPropertyDateTime* stop = nullptr);
};

#endif // DELEGATES_H
