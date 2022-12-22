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
#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Widgets/DateTime/widgetsdatetimepopuppicker.h"
#include "WidgetsModule/Widgets/DateTime/widgetsdatetimewidget.h"

DelegatesComboboxCustomViewModel::DelegatesComboboxCustomViewModel(const ModelGetter& getter, QObject* parent)
    : Super([]()-> QStringList { return {}; }, parent)
    , m_getter(getter)
{}

QWidget* DelegatesComboboxCustomViewModel::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex&) const
{
    QComboBox* comboBox = new QComboBox(parent);
    auto* model = m_getter();
    WidgetComboboxWrapper(comboBox).Make([this, comboBox, model](const WidgetComboboxWrapper& wrapper){
        wrapper.CreateCompleter(model, [this, comboBox](const QModelIndex&){
            if(comboBox->lineEdit() != nullptr) {
                return;
            }
            auto* nonConstThis = const_cast<DelegatesComboboxCustomViewModel*>(this);
            emit nonConstThis->commitData(comboBox);
            emit nonConstThis->closeEditor(comboBox);
        });
        wrapper.BlockWheel();
    });
    return comboBox;
}

DelegatesCombobox::DelegatesCombobox(QObject* parent)
    : Super(parent)
    , m_aligment(Qt::AlignCenter)
{

}

DelegatesCombobox::DelegatesCombobox(const std::function<QStringList ()>& valuesExtractor, QObject* parent)
    : Super(parent)
    , m_valuesExtractor(valuesExtractor)
    , m_aligment(Qt::AlignCenter)
{}

QWidget* DelegatesCombobox::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& ) const
{
    QComboBox* comboBox = new QComboBox(parent);
    WidgetWrapper(comboBox).BlockWheel();
    comboBox->addItems(m_valuesExtractor());
    for (int i = 0; i < comboBox->count() ; ++i) {
        comboBox->setItemData(i, m_aligment, Qt::TextAlignmentRole);
    }
    connect(comboBox, QOverload<qint32>::of(&QComboBox::activated), [this, comboBox](qint32){
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
    OnAboutToSetModelData(combo, index);
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
    auto* spin = new WidgetsSpinBoxWithCustomDisplay(parent);
    OnEditorAboutToBeShown(spin, index);
    connect(spin, QOverload<qint32>::of(&QSpinBox::valueChanged), [this, index](int value){
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
    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, index](double value){
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

QString DelegatesDateTime::displayText(const QVariant& value, const QLocale&) const
{
    return value.toString();
}

DelegatesCheckBox::DelegatesCheckBox(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

void DelegatesCheckBox::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value = index.data(Qt::EditRole);
    if(value.isNull() || index.data(Qt::CheckStateRole).isValid()){
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
    Super::paint(painter, option, index);
    QApplication::style()->drawControl(QStyle::CE_CheckBox,&checkboxstyle, painter, option.widget);
}

bool DelegatesCheckBox::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if(index.flags().testFlag(Qt::ItemIsEditable)) {
        if(event->type() == QEvent::MouseButtonRelease){
            model->setData(index, !model->data(index, Qt::EditRole).toBool());
            event->accept();
        } else if(event->type() == QEvent::MouseButtonDblClick){
            event->accept();
            return true;
        }
    }
    return Super::editorEvent(event, model, option, index);
}

DelegatesDateTimePicker::DelegatesDateTimePicker(QObject* parent)
    : Super(parent)
{

}

QWidget* DelegatesDateTimePicker::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto widget = new WidgetsDatetimePopupPicker(parent);
    widget->Locale = SharedSettings::GetInstance().LanguageSettings.ApplicationLocale.Native();
    widget->GetDateTimeWidget()->CurrentDateTime = index.model()->data(index, Qt::EditRole).toDateTime();
    OnEditorAboutToBeShown(widget, index);
    widget->OnDataCommit.Connect(CONNECTION_DEBUG_LOCATION, [this, widget]{
        auto* nonConst = const_cast<DelegatesDateTimePicker*>(this);
        nonConst->emit commitData(widget);
    });

    widget->OnCloseEditor.Connect(CONNECTION_DEBUG_LOCATION, [this, widget]{
        auto* nonConst = const_cast<DelegatesDateTimePicker*>(this);
        nonConst->emit closeEditor(widget);
    });
    return widget;
}

void DelegatesDateTimePicker::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
{
    editor->setGeometry(option.rect);
}

void DelegatesDateTimePicker::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* widget = dynamic_cast<WidgetsDatetimePopupPicker*>(editor);
    Q_ASSERT(widget != nullptr);
    model->setData(index, widget->GetDateTimeWidget()->CurrentDateTime.Native(), Qt::EditRole);
}

void DateTimeRangeAttachment::Attach(DelegatesDateTimePicker* delegate, const QPair<int,int>& columns, LocalPropertyDateTime* start, LocalPropertyDateTime* stop)
{
    delegate->OnEditorAboutToBeShown.Connect(CONNECTION_DEBUG_LOCATION, [columns, start, stop](QWidget* w, const QModelIndex& index){
        auto* picker = reinterpret_cast<WidgetsDatetimePopupPicker*>(w);
        auto& currentDateTime = picker->GetDateTimeWidget()->CurrentDateTime;
        currentDateTime = index.data(Qt::EditRole).toDateTime();

        if(currentDateTime.IsRealTime()) {
            if(index.column() == columns.second) {
                auto startDateTime = index.model()->index(index.row(), columns.first).data(Qt::EditRole).toDateTime();
                currentDateTime = startDateTime.isValid() ? startDateTime : QDateTime::currentDateTime();
            } else {
                auto startDateTime = index.model()->index(index.row() - 1, columns.second).data(Qt::EditRole).toDateTime();
                currentDateTime = startDateTime.isValid() ? startDateTime : QDateTime::currentDateTime();
            }
        }
        currentDateTime.SetMinMax(*start, *stop);
    });
}

DelegatesComboboxSelector::ValueExtractorHandler DelegatesComboboxSelector::IdExtractor([](const QModelIndex& index){
    return index.data(Qt::EditRole);
});
DelegatesComboboxSelector::ComparatorHandler DelegatesComboboxSelector::IdComparator([](const QModelIndex& tableIndex, const QModelIndex& comboIndex){
    return IdExtractor(tableIndex) == IdExtractor(comboIndex);//прокатит?
});

DelegatesComboboxSelector::DelegatesComboboxSelector(const DelegatesComboboxCustomViewModel::ModelGetter& getter, QObject* parent)
    : Super(getter, parent)
{
    SetCustomComparator(IdComparator);
    SetValueExtractor(IdExtractor);
}

QWidget* DelegatesComboboxSelector::createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex&) const
{
    auto* comboBox = new QComboBox(parent);
    comboBox->setModel(m_getter());
    WidgetComboboxWrapper(comboBox).Make([this, comboBox](const WidgetComboboxWrapper& wrapper){
        wrapper.BlockWheel();
        wrapper.OnActivated().Connect(CONNECTION_DEBUG_LOCATION, [this, comboBox](qint32){
            auto* nonConstThis = const_cast<DelegatesComboboxSelector*>(this);
            emit nonConstThis->commitData(comboBox);
            emit nonConstThis->closeEditor(comboBox);
        });
    });
    return comboBox;
}

void DelegatesComboboxSelector::setEditorData(QWidget* editor, const QModelIndex& tableIndex) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(comboBox != nullptr);
    OnEditorAboutToBeShown(comboBox, tableIndex);
    auto comboboxModel = comboBox->model();
    int dataIndex(-1);
    for(int row(0), count(comboboxModel->rowCount()); row < count; ++row){
        if(m_comparator(tableIndex, comboboxModel->index(row, 0))){
            dataIndex = row;
            break;
        }
    }
    comboBox->setCurrentIndex(dataIndex);
//    ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION, [comboBox]{
//        comboBox->showPopup();
//    });
}

void DelegatesComboboxSelector::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QComboBox* combo = static_cast<QComboBox*>(editor);
    OnAboutToSetModelData(combo, index);
    model->setData(index, m_extractor(combo->model()->index(combo->currentIndex(), 0)));
}
