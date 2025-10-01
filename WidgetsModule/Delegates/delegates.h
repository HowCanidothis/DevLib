#ifndef WIDGETS_DELEGATES_H
#define WIDGETS_DELEGATES_H

#include <QStyledItemDelegate>

#include <SharedModule/internal.hpp>

namespace Delegates {
    void AdjustDialogEditorToCell(QWidget* editor, const QStyleOptionViewItem& option);
}

class QComboBox;
class DelegatesCombobox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesCombobox (QObject* parent);
    DelegatesCombobox (const std::function<QStringList ()>& valuesExtractor, QObject* parent);

    DelegatesCombobox* SetInitializeHandler(const std::function<bool(QComboBox*, const QModelIndex& )>& handler) { m_initializeHandler = handler; return this; }
    DelegatesCombobox* SetAlignment(Qt::AlignmentFlag alignment) { m_aligment = alignment; return this; }
    DelegatesCombobox* SetDrawCombobox(bool draw){ m_drawCombobox = draw; return this; }
    DelegatesCombobox* SetDrawRichText(bool draw) { m_drawRichText = draw; return this; }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    CommonDispatcher<QComboBox*, const QModelIndex&> OnEditorAboutToBeShown;
    CommonDispatcher<QComboBox*, const QModelIndex&> OnAboutToSetModelData;

protected:
    std::function<QStringList ()> m_valuesExtractor;
    Qt::AlignmentFlag m_aligment;
    bool m_drawCombobox;
    bool m_drawRichText;
    std::function<bool(QComboBox*, const QModelIndex& )> m_initializeHandler;
};

class DelegatesComboboxCustomViewModel : public DelegatesCombobox
{
    using Super = DelegatesCombobox;
public:
    using ModelGetter = std::function<QAbstractItemModel* ()>;
    DelegatesComboboxCustomViewModel(const ModelGetter& getter, QObject* parent, bool completer = true);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex&) const override;

protected:
    bool m_includeCompleter;
    ModelGetter m_getter;
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

class DelegatesColor : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesColor(QObject* parent)
        : Super(parent)
    {}

    QString displayText(const QVariant& value, const QLocale& locale) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    CommonDispatcher<class QWidget*, const QModelIndex&> OnEditorAboutToBeShown;
    CommonDispatcher<QDateTime, const QModelIndex&> OnEditorValueChanged;

private:
    mutable class WidgetsDialog* m_editor;
};

class DelegatesLineEdit : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesLineEdit(QObject* parent);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    CommonDispatcher<class QWidget*, const QModelIndex&> OnEditorAboutToBeShown;
};

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
    CommonDispatcher<QVariant&, const QModelIndex&> OnAboutToSetData;
};

class DelegatesDate : public DelegatesDateTime
{
    using Super = DelegatesDateTime;
public:
    DelegatesDate(QObject* parent)
        : Super(parent)
    {}
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    mutable LocalPropertyDateTime m_dateTime;
    mutable class WidgetsDateTimeWidget* m_editor;
    mutable bool m_accepted;
};

class DelegatesCheckBox : public QStyledItemDelegate
{
    Q_OBJECT
    using Super = QStyledItemDelegate;
public:
    DelegatesCheckBox (QObject* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    QString displayText(const QVariant& value, const QLocale& locale) const override;
};

class DelegatesDateTimePicker : public DelegatesDateTime
{
    using Super = DelegatesDateTime;
public:
    DelegatesDateTimePicker(QObject* parent);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override;
    void setEditorData(QWidget* , const QModelIndex& ) const override {}
    void setModelData(QWidget* , QAbstractItemModel* model, const QModelIndex& index) const override;

    static void AttachDateTimeRange(DelegatesDateTimePicker* delegate, const QPair<int, int>& columns, LocalPropertyDateTime* start = nullptr, LocalPropertyDateTime* stop = nullptr);

private:
    mutable bool m_accepted;
};

class DelegatesTimePicker : public DelegatesDateTime
{
    using Super = DelegatesDateTime;
public:
    DelegatesTimePicker(QObject* parent)
        : Super(parent)
    {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override;
    void setEditorData(QWidget* , const QModelIndex& ) const override {}
    void setModelData(QWidget* , QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    mutable class WidgetsTimeWidget* m_editor;
};

class WidgetsMonthPopupPicker;
class DelegatesMonth : public DelegatesDateTime
{
    using Super = DelegatesDateTime;
public:
    DelegatesMonth(QObject* parent)
        : Super(parent)
    {}
    using DisplayHandler = std::function<QString(const QVariant&, const QLocale&)>;
    void SetDiplayText(const DisplayHandler& displayHandler);
    QString displayText(const QVariant& value, const QLocale& locale) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override;
    void setEditorData(QWidget* w, const QModelIndex& ) const override;
    void setModelData(QWidget* , QAbstractItemModel* model, const QModelIndex& index) const override;
private:
    DisplayHandler m_displayHandler;
    mutable bool m_apply;
    mutable class WidgetsMonthPicker* m_editor;
};

#endif // DELEGATES_H
