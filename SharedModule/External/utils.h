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

class MenuWrapper
{
public:
    MenuWrapper(QWidget* widget)
        : m_widget(widget)
    {}

    const MenuWrapper& Make(const std::function<void (const MenuWrapper&)>& handler) const { handler(*this); return *this; }
    ActionWrapper AddAction(const QString& title, const std::function<void ()>& handle) const;
    ActionWrapper AddAction(const QString &title, const std::function<void (QAction*)> &handle) const;
    ActionWrapper AddCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler) const;
    ActionWrapper AddColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler) const;
    ActionWrapper AddDoubleAction(const QString& title, double value, const std::function<void (double value)>& handler) const;
    ActionWrapper AddSeparator() const;
    class QMenu* AddPreventedFromClosingMenu(const QString& title) const;
    static QMenu* CreatePreventedFromClosingMenu(const QString& title);
    QMenu* AddMenu(const QString& label) const;

    QMenu* GetMenu() const { return reinterpret_cast<QMenu*>(m_widget); }

private:
    QWidget* m_widget;
};

_Export void forEachModelIndex(const QAbstractItemModel* model, QModelIndex parent, const std::function<bool (const QModelIndex& index)>& function);


#endif

#endif // UTILS_H
