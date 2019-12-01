#include "propertiesdelegatefactory.h"

#ifdef QT_GUI_LIB

#include <QComboBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QColorDialog>
#include <QPainter>

#include "PropertiesModule/propertiesmodel.h"
#include "PropertiesModule/property.h"
#include "propertiesstyleddelegatelistener.h"

PropertiesDelegateFactory::PropertiesDelegateFactory()
{

}

PropertiesDelegateFactory::~PropertiesDelegateFactory()
{

}

void PropertiesDelegateFactory::SetFactory(PropertiesDelegateFactory* factory)
{
    delete currentFactory();
    currentFactory() = factory;
}

const PropertiesDelegateFactory&PropertiesDelegateFactory::Instance()
{
    return *currentFactory();
}

QWidget*PropertiesDelegateFactory::CreateEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
    QVariant delegateData = index.data(PropertiesModel::RoleDelegateData);
    QVariant delegateValue = index.data(PropertiesModel::RoleDelegateValue);
    switch (delegateValue.toInt()) {
    case Property::DelegateNamedUInt: {
        QComboBox* result = new QComboBox(parent);
        result->addItems(delegateData.toStringList());
        return result;
    }
    case Property::DelegateColor: {
        QColorDialog* result = new QColorDialog(parent);
        result->setCurrentColor(index.data().value<QColor>());
        result->setOption(QColorDialog::ShowAlphaChannel);
        return result;
    }
    case Property::DelegateRect:
    case Property::DelegatePositionXYZ: {

    }
    default:
        break;
    }

    return nullptr;
}

bool PropertiesDelegateFactory::SetModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    switch (index.data(PropertiesModel::RoleDelegateValue).toInt()) {
    case Property::DelegateNamedUInt:
        if(auto e = qobject_cast<QComboBox*>(editor)) {
            model->setData(index, e->currentIndex());
        }
        return true;
    case Property::DelegateRect:
        if(auto e = qobject_cast<QLineEdit*>(editor)) {
            QRegExp regExp(R"((\d+),(\d+),(-?\d+),(-?\d+))");
            if(regExp.exactMatch(e->text())) {
                auto x = regExp.cap(1).toInt();
                auto y = regExp.cap(2).toInt();
                auto width = regExp.cap(3).toInt();
                auto height = regExp.cap(4).toInt();
                model->setData(index, Rect(x,y,width,height));
            }
        }
    case Property::DelegateColor:
        if(auto e = qobject_cast<QColorDialog*>(editor)) {
            if(e->selectedColor().isValid()) {
                model->setData(index, e->selectedColor());
            }
        }
        return true;
    default:
        break;
    }
    return false;
}

bool PropertiesDelegateFactory::SetEditorData(QWidget* editor, const QModelIndex& index, const QStyledItemDelegate* delegate) const
{
    switch (index.data(PropertiesModel::RoleDelegateValue).toInt()) {
    case Property::DelegateNamedUInt:
        if(auto e = qobject_cast<QComboBox*>(editor)) {
            e->setCurrentIndex(index.data().toInt());
            auto listener = new PropertiesStyledDelegateListener(e,index,delegate);
            QObject::connect(e, SIGNAL(currentIndexChanged(int)), listener, SLOT(onEditorValueChanged()));
        }
        return true;
    case Property::DelegateRect:
        if(auto e = qobject_cast<QLineEdit*>(editor)) {
            QString text;
            DisplayText(text, index.data(), QLocale());
            e->setText(text);
        }
        return true;
    default:
        break;
    }
    return false;
}

bool PropertiesDelegateFactory::DisplayText(QString& text, const QVariant& value, const QLocale&) const
{
    switch (value.type()) {
    case QVariant::Rect: {
        auto rect = value.toRect();
        text = QString("%1,%2,%3,%4").arg(QString::number(rect.x()),
                                          QString::number(rect.y()),
                                          QString::number(rect.width()),
                                          QString::number(rect.height())
                                          );
    }
        return true;
    default:
        break;
    }

    return false;
}

bool PropertiesDelegateFactory::Paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QVariant delegateValue = index.data(PropertiesModel::RoleDelegateValue);
    if(delegateValue.toInt() == Property::DelegateColor) {
        painter->fillRect(option.rect, index.data().value<QColor>());
        return true;
    }

    return false;
}

PropertiesDelegateFactory*&PropertiesDelegateFactory::currentFactory()
{
    static PropertiesDelegateFactory* currentFactory = new PropertiesDelegateFactory();
    return currentFactory;
}

#endif
