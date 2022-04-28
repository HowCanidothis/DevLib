#ifndef UTILS_H
#define UTILS_H

#include <functional>

#ifdef QT_GUI_LIB

#include <QAbstractItemModel>
#include "SharedModule/internal.hpp"

class ActionWrapper
{
public:
    ActionWrapper(QAction* action);

    ActionWrapper& Make(const std::function<void (ActionWrapper&)>& handler);
    ActionWrapper& SetShortcut(const QKeySequence& keySequence);
    ActionWrapper& SetText(const QString& text);

#ifdef PROPERTIES_LIB
    LocalPropertyBool& ActionVisibility();
    LocalPropertyBool& ActionEnablity();
    SharedPointer<class TranslatedString> ActionText();
#endif

    QAction* operator->() const { return m_action; }
    operator QAction*() const { return m_action; }

private:
#ifdef PROPERTIES_LIB
    template<class Property, typename ... Args>
    SharedPointer<Property> getOrCreateProperty(const char* propName, const std::function<void (QAction*, const Property&)>& handler, Args... args) const
    {
        SharedPointer<Property> property = m_action->property(propName).value<SharedPointer<Property>>();
        if(property == nullptr) {
            auto* action = m_action;
            property = ::make_shared<Property>(args...);
            m_action->setProperty(propName, QVariant::fromValue(property));
            auto* pProperty = property.get();
            property->OnChanged.ConnectAndCall(this, [action, handler, pProperty]{ handler(action, *pProperty); });
            property->SetSetterHandler(ThreadHandlerMain);
        }
        return property;
    }
#endif

private:
    QAction* m_action;
};

#define ActionWrapperMake(capture, action) \
    Make([capture](ActionWrapper& wrapper) { action })

_Export void forEachModelIndex(const QAbstractItemModel* model, QModelIndex parent, const std::function<bool (const QModelIndex& index)>& function);
_Export class ActionWrapper createAction(const QString& title, const std::function<void ()>& handle, QWidget* menu);
_Export class ActionWrapper createAction(const QString &title, const std::function<void (QAction*)> &handle, QWidget* menu);
_Export class ActionWrapper createCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler, QWidget* menu);
_Export class ActionWrapper createColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler, QWidget* menu);
_Export class ActionWrapper createDoubleAction(const QString& title, double value, const std::function<void (double value)>& handler, QWidget* menu);
_Export class QMenu* createPreventedFromClosingMenu(const QString& title, QMenu* menu = nullptr);
using QtEventFilterHandler = std::function<bool (QObject *watched, QEvent *event)>;
_Export void installEventFilter(QObject* target, const QtEventFilterHandler& eventFilter);

#endif

#endif // UTILS_H
