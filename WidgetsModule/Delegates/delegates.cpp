#include "delegates.h"

#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <SharedModule/internal.hpp>

DelegatesCombobox::DelegatesCombobox(Qt::AlignmentFlag aligment, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_aligment(aligment)
{

}

DelegatesCombobox::DelegatesCombobox(const QStringList& valuesList, Qt::AlignmentFlag aligment, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_values(valuesList)
    , m_aligment(aligment)
{

}

void DelegatesCombobox::setValues(const QStringList& valuesList) {
    m_values = valuesList;
}

void DelegatesCombobox::setAligment(const Qt::AlignmentFlag& aligment)
{
    m_aligment = aligment;
}

QWidget* DelegatesCombobox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& ) const
{
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItems(m_values);
    for (int i = 0; i < comboBox->count() ; ++i) {
        comboBox->setItemData(i, m_aligment, Qt::TextAlignmentRole);
    }
    return comboBox;
}

void DelegatesCombobox::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QString data = index.model()->data(index, Qt::EditRole).toString();

    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(comboBox != nullptr);
    OnEditorAboutToBeShown(comboBox, index);
    comboBox->setCurrentText(data);
}

void DelegatesCombobox::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QComboBox* combo = static_cast<QComboBox*>(editor);
    const QString& text = combo->currentText();

    model->setData(index, text, Qt::EditRole);
}

void DelegatesCombobox::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& ) const {
    editor->setGeometry(option.rect);
}


DelegatesIntSpinBox::DelegatesIntSpinBox(int min, int max, int step, QObject* parent)
    : QStyledItemDelegate(parent)
      , m_min(min)
      , m_max(max)
      , m_step(step)
      , m_editHandler([](QAbstractItemModel*, const QModelIndex&)->bool {return true;})//todo static default
{
    
}

QWidget* DelegatesIntSpinBox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    auto* spin = new QSpinBox (parent);
    spin->setRange(m_min, m_max);
    spin->setSingleStep(m_step);
    OnEditorAboutToBeShown(spin, index);
    connect(spin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this, index](int value){
        OnEditorValueChanged(value, index);
    });
    return spin;
}

void DelegatesIntSpinBox::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto data = index.model()->data(index, Qt::EditRole).toInt();
    auto* spin = qobject_cast<QSpinBox*>(editor);
    Q_ASSERT(spin != nullptr);
    spin->setValue(data);
}

void DelegatesIntSpinBox::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto* spin = static_cast<QSpinBox*>(editor);
    const auto& val = ::clamp(spin->value(), spin->minimum(), spin->maximum());
    
    model->setData(index, val, Qt::EditRole);
}

void DelegatesIntSpinBox::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& ) const {
    editor->setGeometry(option.rect);
}

bool DelegatesIntSpinBox::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if(event->type() == QEvent::MouseButtonDblClick){
        auto isEditable = m_editHandler(model, index);
        if (!isEditable){///block
            return true;
        }
    }
    return Super::editorEvent(event, model, option, index);
}

void DelegatesIntSpinBox::SetEditHandler(const std::function<bool (QAbstractItemModel*, const QModelIndex&)>& handler){
    m_editHandler = handler;
}

void DelegatesIntSpinBox::SetRange(int min, int max){
    m_min = min;
    m_max = max;
}

DelegatesDoubleSpinBox::DelegatesDoubleSpinBox(double min, double max, double step, int precision, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_precision(precision)
    , m_min(min)
    , m_max(max)
    , m_step(step)
    , m_editHandler([](QAbstractItemModel*, const QModelIndex&)->bool {return true;})//todo static default
{
    
}

QWidget* DelegatesDoubleSpinBox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    QDoubleSpinBox* spin = new QDoubleSpinBox (parent);
    spin->setDecimals(m_precision);
    spin->setRange(m_min, m_max);
    spin->setSingleStep(m_step);
    OnEditorAboutToBeShown(spin, index);
    connect(spin, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this, index](double value){
        OnEditorValueChanged(value, index);
    });
    return spin;
}

void DelegatesDoubleSpinBox::setEditorData(QWidget* editor, const QModelIndex& index) const {
    double data = index.model()->data(index, Qt::EditRole).toDouble();

    QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(editor);
    Q_ASSERT(spin != nullptr);
    spin->setValue(data);
}

void DelegatesDoubleSpinBox::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QDoubleSpinBox* spin = static_cast<QDoubleSpinBox*>(editor);
    const double& val = ::clamp(spin->value(), spin->minimum(), spin->maximum());

    model->setData(index, val, Qt::EditRole);
}

void DelegatesDoubleSpinBox::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& ) const {
    editor->setGeometry(option.rect);
}

bool DelegatesDoubleSpinBox::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if(event->type() == QEvent::MouseButtonDblClick){
        auto isEditable = m_editHandler(model, index);
        if (!isEditable){///block
            return true;
        }
    }
    return Super::editorEvent(event, model, option, index);
}

void DelegatesDoubleSpinBox::SetEditHandler(const std::function<bool (QAbstractItemModel*, const QModelIndex&)>& handler){
    m_editHandler = handler;
}

void DelegatesDoubleSpinBox::SetRange(double min, double max){
    m_min = min;
    m_max = max;
}

DelegatesDateTime::DelegatesDateTime(QObject* parent)
    : QStyledItemDelegate(parent)
	, m_displayFormat("MM/dd/yy hh:mm:ss")
{
    
}

QWidget* DelegatesDateTime::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    auto* editor = new QDateTimeEdit(parent);
    editor->setDisplayFormat(m_displayFormat);
    editor->setTimeSpec(Qt::UTC);
	OnEditorAboutToBeShown(editor, index);
    connect(editor,&QDateTimeEdit::dateTimeChanged, [this, index](const QDateTime&dateTime){
        OnEditorValueChanged(dateTime, index);
    });
    return editor;
}

void DelegatesDateTime::setEditorData(QWidget* editor, const QModelIndex& index) const {
    const auto& dateTime = index.model()->data(index, Qt::EditRole).toDateTime();
	
    QDateTimeEdit* dt = qobject_cast<QDateTimeEdit*>(editor);
    Q_ASSERT(dt != nullptr);
	dt->setTimeSpec(dateTime.timeSpec());
    dt->setDateTime(dateTime);
}

void DelegatesDateTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QDateTimeEdit* dt = static_cast<QDateTimeEdit*>(editor);
    model->setData(index, dt->dateTime(), Qt::EditRole);
}

void DelegatesDateTime::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& ) const {
    editor->setGeometry(option.rect);
}

DelegatesCheckBox::DelegatesCheckBox(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

void DelegatesCheckBox::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    bool data = index.model()->data(index, Qt::DisplayRole).toBool();
    
    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator,&checkboxstyle);
    
    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() +
                                 option.rect.width()/2 - checkbox_rect.width()/2);

    if(data) {
        checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
    } else {
        checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;
    }

    QApplication::style()->drawControl(QStyle::CE_CheckBox,&checkboxstyle, painter);
}

bool DelegatesCheckBox::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if(event->type() == QEvent::MouseButtonRelease){
        model->setData(index, !model->data(index).toBool());
        event->accept();
    } else if(event->type() == QEvent::MouseButtonDblClick){
        event->accept();
        return true;
    }
    return Super::editorEvent(event, model, option, index);
}
////////////////////////////////////
//DelegatesIconBox::DelegatesIconBox(const QIcon& icon, QObject* parent)
//    : QStyledItemDelegate(parent)
//	, m_icon(icon)
//    , m_editHandler([](QAbstractItemModel*, const QModelIndex&)->bool {return true;})//todo static default
//{
    
//}

//QWidget* DelegatesIconBox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
//{
//    auto* label = new QLabel (parent);
//    OnEditorAboutToBeShown(label, index);
//    return spin;
//}

//void DelegatesIconBox::setEditorData(QWidget* editor, const QModelIndex& index) const {
//    auto data = index.model()->data(index, Qt::EditRole).toInt();
//    auto* spin = qobject_cast<QSpinBox*>(editor);
//    Q_ASSERT(spin != nullptr);
//    spin->setValue(data);
//}

//void DelegatesIconBox::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
//    auto* spin = static_cast<QSpinBox*>(editor);
//    const auto& val = ::clamp(spin->value(), spin->minimum(), spin->maximum());
    
//    model->setData(index, val, Qt::EditRole);
//}

//void DelegatesIconBox::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& ) const {
//    editor->setGeometry(option.rect);
//}

//bool DelegatesIconBox::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
//{
//    if(event->type() == QEvent::MouseButtonDblClick){
//        auto isEditable = m_editHandler(model, index);
//        if (!isEditable){///block
//            return true;
//        }
//    }
//    return Super::editorEvent(event, model, option, index);
//}
