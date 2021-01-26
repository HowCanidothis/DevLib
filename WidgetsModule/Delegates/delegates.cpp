#include "delegates.h"

#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <SharedModule/internal.hpp>

DelegatesCombobox::DelegatesCombobox(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

DelegatesCombobox::DelegatesCombobox(const QStringList& valuesList, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_values(valuesList)
{

}

void DelegatesCombobox::setValues(const QStringList& valuesList) {
    m_values = valuesList;
}

QWidget* DelegatesCombobox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& ) const
{
    QComboBox* combo = new QComboBox(parent);
    combo->addItems(m_values);
    return combo;
}

void DelegatesCombobox::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QString data = index.model()->data(index, Qt::EditRole).toString();

    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(comboBox != nullptr);
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
    const double& val = spin->value();

    model->setData(index, val, Qt::EditRole);
}

void DelegatesDoubleSpinBox::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& ) const {
    editor->setGeometry(option.rect);
}

QString DelegatesDoubleSpinBox::displayText(const QVariant& value, const QLocale&) const
{
    return QString::number(value.toDouble(), 'd', m_precision);
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
{
    
}

QWidget* DelegatesDateTime::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    auto editor = new QDateTimeEdit(parent);
    editor->setDisplayFormat("MM/dd/yy hh:mm:ss");
    editor->setTimeSpec(Qt::UTC);
    connect(editor,&QDateTimeEdit::dateTimeChanged, [this, index](const QDateTime&dateTime){
        emit const_cast<DelegatesDateTime*>(this)->dateTimeChanged(dateTime, index);
    });
    return editor;
}

void DelegatesDateTime::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto varData = index.model()->data(index, Qt::EditRole);
    //        auto data = QDateTime::fromString(strData.toString(), "dd.mm.yy hh.mm");
    auto data = varData.toDateTime();

    QDateTimeEdit* dt = qobject_cast<QDateTimeEdit*>(editor);
    Q_ASSERT(dt != nullptr);
    dt->setDateTime(data);
}

void DelegatesDateTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QDateTimeEdit* dt = static_cast<QDateTimeEdit*>(editor);
    const auto& val = dt->dateTime();

    model->setData(index, val, Qt::EditRole);
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
