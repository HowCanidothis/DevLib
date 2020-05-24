#include "propertiesdelegate.h"

#include <QPainter>
#include <QDoubleSpinBox>
#include <QComboBox>

#include "PropertiesModule/property.h"
#include "propertiesdelegatefactory.h"
#include "propertiesstyleddelegatelistener.h"

PropertiesDelegate::PropertiesDelegate(QObject* parent)
    : Super(parent)
    , m_gradientLeft(0x567dbc)
    , m_gradientRight(0x6ea1f1)
    , m_gradientRightBorder(0.7)
{}

void PropertiesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if(index.data(Property::RoleHeaderItem).toBool()){
        QRect orect = option.rect;
        painter->setPen(Qt::NoPen);
        QRect rowRect(0,option.rect.y(),option.widget->width(),orect.height());
        QLinearGradient lg(0,rowRect.y(), rowRect.width(),rowRect.y());
        lg.setColorAt(0, m_gradientLeft);
        lg.setColorAt(m_gradientRightBorder, m_gradientRight);
        painter->setBrush(lg);
        if(!index.column())
            painter->drawRect(orect.adjusted(-orect.x(),0,0,0));
        else
            painter->drawRect(orect);

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const QWidget *widget = option.widget;

        if(float(option.rect.x()) / widget->width() < 0.5)
            opt.palette.setColor(QPalette::Text,Qt::white);
        else
            opt.palette.setColor(QPalette::Text,Qt::red);
        if(index.column()) {
            opt.text = "";
        }

        widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    }
    else if(!PropertiesDelegateFactory::Instance().Paint(painter, option, index)) {
        QStyledItemDelegate::paint(painter,option,index);
    }
}

QWidget* PropertiesDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QVariant data = index.data(Qt::EditRole);

    if(auto editor = PropertiesDelegateFactory::Instance().CreateEditor(parent, option, index)) {
        return editor;
    }

    switch (data.type()) {
    case QVariant::Bool: {
        QComboBox* result = new QComboBox(parent);
        result->addItems({ tr("false"), tr("true") });
        result->setFocusPolicy(Qt::StrongFocus);
        return result;
    }
    case QVariant::UInt:
    case QVariant::Int: {
        QSpinBox* result = new QSpinBox(parent);
        result->setValue(data.toInt());
        result->setMinimum(index.data(Property::RoleMinValue).toInt());
        result->setMaximum(index.data(Property::RoleMaxValue).toInt());
        result->setFocusPolicy(Qt::StrongFocus);
        return result;
    }
    case QVariant::Double:
    case QMetaType::Float: {
        QDoubleSpinBox* result = new QDoubleSpinBox(parent);
        result->setValue(data.toDouble());
        result->setMinimum(index.data(Property::RoleMinValue).toDouble());
        result->setMaximum(index.data(Property::RoleMaxValue).toDouble());
        auto singleStep = (result->maximum() - result->minimum()) / 100.0;
        singleStep = (singleStep > 1.0) ? 1.0 : singleStep;
        result->setSingleStep(singleStep);
        result->setFocusPolicy(Qt::StrongFocus);
        return result;
    }
    default:
        return Super::createEditor(parent, option, index);
    }
}

void PropertiesDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if(PropertiesDelegateFactory::Instance().SetEditorData(editor, index, this)) {
        return;
    }

    Super::setEditorData(editor, index);
    if(auto e = qobject_cast<QSpinBox*>(editor)) {
        auto listener = new PropertiesStyledDelegateListener(e,index,this);
        connect(e, SIGNAL(valueChanged(int)), listener, SLOT(onEditorValueChanged()));
    }
    else if(auto e = qobject_cast<QDoubleSpinBox*>(editor)) {
        auto listener = new PropertiesStyledDelegateListener(e,index,this);
        connect(e, SIGNAL(valueChanged(double)), listener, SLOT(onEditorValueChanged()));
    } else if(auto e = qobject_cast<QComboBox*>(editor)) {
        auto listener = new PropertiesStyledDelegateListener(e,index,this);
        connect(e, SIGNAL(currentIndexChanged(int)), listener, SLOT(onEditorValueChanged()));
    }
}

void PropertiesDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if(PropertiesDelegateFactory::Instance().SetModelData(editor, model, index)) {
        return;
    }

    Super::setModelData(editor, model, index);
}

QString PropertiesDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    QString result;
    if(PropertiesDelegateFactory::Instance().DisplayText(result, value, locale)) {
        return result;
    }
    return Super::displayText(value, locale);
}

void PropertiesDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if(editor->windowFlags() & Qt::Window) {
        auto* parent = editor->parentWidget();
        const auto& geometry = parent->geometry();
        editor->setGeometry(QRect(parent->mapToGlobal(geometry.topLeft()), parent->mapToGlobal(geometry.bottomRight())));
        return;

    }

    return Super::updateEditorGeometry(editor, option, index);
}
