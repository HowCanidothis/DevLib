#include "delegates.h"

#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <SharedModule/internal.hpp>

#include "WidgetsModule/Widgets/widgetsspinboxwithcustomdisplay.h"

DelegatesComboboxCustomViewModel::DelegatesComboboxCustomViewModel(const ModelGetter& getter, QObject* parent)
    : Super([]()-> QStringList { return {}; }, parent)
    , m_getter(getter)
{}

QWidget* DelegatesComboboxCustomViewModel::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex&) const
{
    QComboBox* comboBox = new QComboBox(parent);
    auto* model = m_getter();
    comboBox->setModel(model);
    connect(comboBox, static_cast<void (QComboBox::*)(qint32)>(&QComboBox::activated), [this, comboBox](qint32){
        if(comboBox->lineEdit() != nullptr) {
            return;
        }
        auto* nonConstThis = const_cast<DelegatesComboboxCustomViewModel*>(this);
        emit nonConstThis->commitData(comboBox);
        emit nonConstThis->closeEditor(comboBox);
    });

    return comboBox;
}

DelegatesCombobox::DelegatesCombobox(const std::function<QStringList ()>& valuesExtractor, QObject* parent)
    : Super(parent)
    , m_valuesExtractor(valuesExtractor)
    , m_aligment(Qt::AlignCenter)
{}

QWidget* DelegatesCombobox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& ) const
{
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItems(m_valuesExtractor());
    for (int i = 0; i < comboBox->count() ; ++i) {
        comboBox->setItemData(i, m_aligment, Qt::TextAlignmentRole);
    }
    connect(comboBox, static_cast<void (QComboBox::*)(qint32)>(&QComboBox::activated), [this, comboBox](qint32){
        if(comboBox->lineEdit() != nullptr) {
            return;
        }
        auto* nonConstThis = const_cast<DelegatesCombobox*>(this);
        emit nonConstThis->commitData(comboBox);
        emit nonConstThis->closeEditor(comboBox);
    });
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


DelegatesIntSpinBox::DelegatesIntSpinBox(QObject* parent)
    : QStyledItemDelegate(parent)
      , m_editHandler([](QAbstractItemModel*, const QModelIndex&)->bool {return true;})//todo static default
{
    
}

QWidget* DelegatesIntSpinBox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    auto* spin = new QSpinBox (parent);
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
    
    bool accept = true;
    OnEditingFinished(val, index, accept);
    if(accept) {
        model->setData(index, val, Qt::EditRole);
    }
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

DelegatesDoubleSpinBox::DelegatesDoubleSpinBox(QObject* parent)
    : QStyledItemDelegate(parent)
    , m_editHandler([](QAbstractItemModel*, const QModelIndex&)->bool {return true;})//todo static default
{
}

QWidget* DelegatesDoubleSpinBox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    auto* spin = new WidgetsDoubleSpinBoxWithCustomDisplay (parent);
    spin->MakeOptional();
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
    WidgetsDoubleSpinBoxWithCustomDisplay* spin = static_cast<WidgetsDoubleSpinBoxWithCustomDisplay*>(editor);
    double val = spin->value();

    bool accept = true;
    OnEditingFinished(val, index, accept);
    if(accept) {
        model->setData(index, spin->IsValid() ? val : QVariant(), Qt::EditRole);
    }
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

DelegatesDateTime::DelegatesDateTime(QObject* parent)
    : QStyledItemDelegate(parent)
{
    
}

QWidget* DelegatesDateTime::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& index) const
{
    auto* editor = new QDateTimeEdit(parent);
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

QString DelegatesDateTime::displayText(const QVariant& value, const QLocale& locale) const
{
	return locale.toString(value.toDateTime(), QLocale::ShortFormat);
}

DelegatesCheckBox::DelegatesCheckBox(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

void DelegatesCheckBox::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value = index.model()->data(index, Qt::DisplayRole);
    if(value.isNull()){
        Super::paint(painter, option, index);
        return;
    }
    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator,&checkboxstyle, option.widget);
    
    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() - checkbox_rect.x() +
                                 option.rect.width()/2 - checkbox_rect.width()/2);

    if(value.toBool()) {
        checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled;
    } else {
        checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled;
    }

    QApplication::style()->drawControl(QStyle::CE_CheckBox,&checkboxstyle, painter, option.widget);
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
