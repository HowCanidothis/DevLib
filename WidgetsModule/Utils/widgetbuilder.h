#ifndef WIDGETBUILDER_H
#define WIDGETBUILDER_H

#include <QStack>

#include "widgethelpers.h"

struct WidgetBuilderLayoutParams
{
    Qt::Orientation Orientation;
    bool AddSpacerToTheEnd;

    WidgetBuilderLayoutParams(Qt::Orientation orientation)
        : Orientation(orientation)
        , AddSpacerToTheEnd(false)
    {}

    WidgetBuilderLayoutParams& AddSpacer() { AddSpacerToTheEnd = true; return *this; }
};

class WidgetBuilder
{
    using FAddDelegate = std::function<void (const FTranslationHandler&, QWidget*)>;
public:
    WidgetBuilder(QWidget* parent, Qt::Orientation layoutOrientation, qint32 margins = 9);
    ~WidgetBuilder();

    WidgetBuilder& StartSplitter(const std::function<void (WidgetBuilder&, QSplitter*)>& handler);
    WidgetBuilder& StartLayout(const WidgetBuilderLayoutParams& params, const std::function<void (WidgetBuilder&)>& handler);
    WidgetBuilder& Make(const std::function<void (WidgetBuilder&)>& handler);
    WidgetBuilder& StartLabeledLayout(QuadTreeF::BoundingRect_Location location, const std::function<void (WidgetBuilder&)>& handler);

    class WidgetsDoubleSpinBoxWithCustomDisplay* AddDoubleSpinBox(const FTranslationHandler& label = TR_NONE);
    class WidgetsSpinBoxWithCustomDisplay* AddSpinBox(const FTranslationHandler& label = TR_NONE);
    WidgetPushButtonWrapper AddButton(const FTranslationHandler& label = TR_NONE);
    WidgetComboboxWrapper AddCombobox(const FTranslationHandler& label = TR_NONE);
    WidgetLineEditWrapper AddLineEdit(const FTranslationHandler& label = TR_NONE);
    WidgetLabelWrapper AddLabel(const FTranslationHandler& label = TR_NONE);
    WidgetCheckBoxWrapper AddCheckBox(const FTranslationHandler& label = TR_NONE);

    WidgetBuilder& AddWidget(QWidget* widget, const FTranslationHandler& label = TR_NONE)
    {
        m_addWidgetDelegate(label, widget);
        return *this;
    }

    template<class T>
    WidgetWrapper AddWidget(const FTranslationHandler& label = TR_NONE)
    {
        auto* result = new T();
        m_addWidgetDelegate(label, result);
        return result;
    }

private:
    FAddDelegate defaultAddDelegate();

private:
    QStack<std::function<void (QWidget*)>> m_addWidgetFunctors;
    FAddDelegate m_addWidgetDelegate;
    bool m_usedDefaultDelegate;
};

#endif // WIDGETBUILDER_H
