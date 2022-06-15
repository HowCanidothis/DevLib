#include "widgetbuilder.h"

#include <QSplitter>
#include <QBoxLayout>
#include <QPushButton>

#include "WidgetsModule/Widgets/widgetsspinboxwithcustomdisplay.h"

WidgetBuilder::WidgetBuilder(QWidget* parent, Qt::Orientation layoutOrientation, qint32 margins)
{
    QBoxLayout* layout;
    if(layoutOrientation == Qt::Vertical) {
        layout = new QVBoxLayout(parent);
    } else {
        layout = new QHBoxLayout(parent);
    }
    layout->setMargin(margins);
    m_addWidgetFunctors.append([layout](QWidget* widget){ layout->addWidget(widget); });
}

WidgetBuilder::~WidgetBuilder()
{
}

WidgetBuilder& WidgetBuilder::StartSplitter(const std::function<void (WidgetBuilder&)>& handler)
{
    auto* splitter = new QSplitter();
    m_addWidgetFunctors.last()(splitter);
    m_addWidgetFunctors.append([splitter](QWidget* w){ splitter->insertWidget(splitter->count(), w); });
    handler(*this);
    m_addWidgetFunctors.pop();
    return *this;
}

WidgetBuilder& WidgetBuilder::StartLayout(Qt::Orientation orientation, const std::function<void (WidgetBuilder&)>& handler)
{
    auto* dummyWidget = new QWidget();
    QBoxLayout* toAdd;
    if(orientation == Qt::Vertical) {
        toAdd = new QVBoxLayout(dummyWidget);
    } else {
        toAdd = new QHBoxLayout(dummyWidget);
    }
    m_addWidgetFunctors.last()(dummyWidget);
    m_addWidgetFunctors.append([toAdd](QWidget* w){ toAdd->addWidget(w); });
    handler(*this);
    m_addWidgetFunctors.pop();
    return *this;
}

WidgetBuilder& WidgetBuilder::Make(const std::function<void (WidgetBuilder&)>& handler)
{
    handler(*this);
    return *this;
}

WidgetsSpinBoxWithCustomDisplay* WidgetBuilder::AddSpinBox()
{
    return (WidgetsSpinBoxWithCustomDisplay*)AddWidget<WidgetsSpinBoxWithCustomDisplay>().GetWidget();
}

WidgetPushButtonWrapper WidgetBuilder::AddButton()
{
    return AddWidget<QPushButton>().Cast<WidgetPushButtonWrapper>();
}
