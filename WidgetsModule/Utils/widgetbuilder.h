#ifndef WIDGETBUILDER_H
#define WIDGETBUILDER_H

#include <QStack>

#include "widgethelpers.h"

class WidgetBuilder
{
public:
    WidgetBuilder(QWidget* parent, Qt::Orientation layoutOrientation, qint32 margins = 9);
    ~WidgetBuilder();

    WidgetBuilder& StartSplitter(const std::function<void (WidgetBuilder&, QSplitter*)>& handler);
    WidgetBuilder& StartLayout(Qt::Orientation orientation, const std::function<void (WidgetBuilder&)>& handler);
    WidgetBuilder& Make(const std::function<void (WidgetBuilder&)>& handler);

    class WidgetsSpinBoxWithCustomDisplay* AddSpinBox();
    WidgetPushButtonWrapper AddButton();

    template<class T>
    WidgetWrapper AddWidget()
    {
        auto* result = new T();
        m_addWidgetFunctors.last()(result);
        return result;
    }

private:
    QStack<std::function<void (QWidget*)>> m_addWidgetFunctors;
};
#endif // WIDGETBUILDER_H
