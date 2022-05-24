#include "utils.h"
#ifdef QT_GUI_LIB
#include <QAction>
#include <QMenu>
#include <QCheckBox>
#include <QWidgetAction>
#include <QPushButton>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QApplication>

#include <PropertiesModule/internal.hpp>

#ifdef WIDGETS_MODULE_LIB
#include "WidgetsModule/Managers/widgetsdialogsmanager.h"
void notifyWidgetsManager(QDialog* dialog) { WidgetsDialogsManager::GetInstance().OnDialogCreated(dialog); }
#else
void notifyWidgetsManager(QDialog*) {}
#endif

ActionWrapper::ActionWrapper(QAction* action)
    : m_action(action)
{

}

ActionWrapper& ActionWrapper::Make(const std::function<void (ActionWrapper&)>& handler)
{
    handler(*this);
    return *this;
}

ActionWrapper& ActionWrapper::SetShortcut(const QKeySequence& keySequence)
{
    m_action->setShortcut(keySequence);
    return *this;
}

ActionWrapper& ActionWrapper::SetText(const QString& text)
{
    m_action->setText(text);
    return *this;
}

#ifdef PROPERTIES_LIB
LocalPropertyBool& ActionWrapper::ActionVisibility()
{
    return *getOrCreateProperty<LocalPropertyBool>("a_visible", [](QAction* action, const LocalPropertyBool& visible){
        action->setVisible(visible);
    }, true);
}

LocalPropertyBool& ActionWrapper::ActionEnablity()
{
    return *getOrCreateProperty<LocalPropertyBool>("a_enable", [](QAction* action, const LocalPropertyBool& visible){
        action->setEnabled(visible);
    }, true);
}

TranslatedStringPtr ActionWrapper::ActionText()
{
    return getOrCreateProperty<TranslatedString>("a_text", [](QAction* action, const TranslatedString& text){
        action->setText(text);
    }, []{ return QString(); });
}

#endif

ActionWrapper MenuWrapper::AddSeparator() const
{
    QAction *action = new QAction(m_widget);
    action->setSeparator(true);
    m_widget->addAction(action);
    return action;
}

ActionWrapper MenuWrapper::AddAction(const QString& title, const std::function<void ()>& handle) const
{
    auto result = new QAction(title, m_widget);
    result->connect(result, &QAction::triggered, handle);
    m_widget->addAction(result);
    return result;
}

ActionWrapper MenuWrapper::AddAction(const QString& title, const std::function<void (QAction*)>& handle) const
{
    auto result = new QAction(title, m_widget);
    result->connect(result, &QAction::triggered, [handle, result]{
        handle(result);
    });
    m_widget->addAction(result);
    return result;
}

ActionWrapper MenuWrapper::AddCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler) const
{
    auto result = new QAction(title, m_widget);
    result->setCheckable(true);
    result->setChecked(checked);
    result->connect(result, &QAction::triggered, [handler, result]{
        handler(result->isChecked());
    });
    m_widget->addAction(result);
    return result;
}

ActionWrapper MenuWrapper::AddColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler) const
{
    static QPixmap pixmap(10,10);
    auto colorAction = AddAction(title, [handler, color](QAction* action){
        QColorDialog dialog(qApp->activeWindow());
        dialog.setModal(true);
        notifyWidgetsManager(&dialog);
        dialog.setCurrentColor(color);
        if(dialog.exec() == QDialog::Accepted) {
            auto result = dialog.currentColor();
            pixmap.fill(result);
            action->setIcon(pixmap);
            handler(result);
        }
    });
    pixmap.fill(color);
    colorAction->setIcon(pixmap);
    return colorAction;
}

ActionWrapper MenuWrapper::AddDoubleAction(const QString& title, double value, const std::function<void (double value)>& handler) const
{
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout();
    layout->setContentsMargins(6,3,6,3);
    widget->setLayout(layout);
    auto* label = new QLabel(title);
    auto* spinBox = new QDoubleSpinBox;
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    layout->addWidget(spinBox);
    spinBox->setValue(value);
    QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [handler](double value) {
        handler(value);
    });
    auto* action = new QWidgetAction(m_widget);
    action->setDefaultWidget(widget);
    m_widget->addAction(action);
    return action;
}

class PreventedFromClosingMenu : public QMenu
{
    using Super = QMenu;
public:
    using Super::Super;

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        QAction *action = activeAction();
        if (action && action->isEnabled()) {
            action->trigger();
        }
        else
            QMenu::mouseReleaseEvent(e);
    }
};

QMenu* MenuWrapper::CreatePreventedFromClosingMenu(const QString& title)
{
    return new PreventedFromClosingMenu(title);
}

QMenu* MenuWrapper::AddPreventedFromClosingMenu(const QString& title) const
{
    auto* result = new PreventedFromClosingMenu(title, m_widget);
    m_widget->addAction(result->menuAction());
    return result;
}

QMenu* MenuWrapper::AddMenu(const QString& label) const
{
    auto* result = new QMenu(label, m_widget);
    m_widget->addAction(result->menuAction());
    return result;
}

void forEachModelIndex(const QAbstractItemModel* model, QModelIndex parent, const std::function<bool (const QModelIndex&)>& function)
{
    auto rowCount = model->rowCount(parent);
    for(int r = 0; r < rowCount; ++r) {
        QModelIndex index = model->index(r, 0, parent);
        if(function(index)) {
            return;
        }
        // here is your applicable code
        if( model->hasChildren(index) ) {
            forEachModelIndex(model, index, function);
        }
    }
}

#endif

