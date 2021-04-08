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

QAction* createAction(const QString& title, const std::function<void ()>& handle, QWidget* menu)
{
    auto result = new QAction(title, menu);
    result->connect(result, &QAction::triggered, handle);
    menu->addAction(result);
    return result;
}

QAction* createAction(const QString& title, const std::function<void (QAction*)>& handle, QWidget* menu)
{
    auto result = new QAction(title, menu);
    result->connect(result, &QAction::triggered, [handle, result]{
        handle(result);
    });
    menu->addAction(result);
    return result;
}

QAction* createCheckboxAction(const QString& title, bool checked, const std::function<void (bool)>& handler, QWidget* menu)
{
    auto result = new QAction(title, menu);
    result->setCheckable(true);
    result->setChecked(checked);
    result->connect(result, &QAction::triggered, [handler, result]{
        handler(result->isChecked());
    });
    menu->addAction(result);
    return result;
}

QAction* createColorAction(const QString& title, const QColor& color, const std::function<void (const QColor& color)>& handler, QWidget* menu)
{
    static QPixmap pixmap(10,10);
    auto* colorAction = createAction(title, [handler, color](QAction* action){
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
    }, menu);
    pixmap.fill(color);
    colorAction->setIcon(pixmap);
    return colorAction;
}

QAction* createDoubleAction(const QString& title, double value, const std::function<void (double value)>& handler, QWidget* menu)
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
    QObject::connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [handler](double value) {
        handler(value);
    });
    auto* action = new QWidgetAction(menu);
    action->setDefaultWidget(widget);
    menu->addAction(action);
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

QMenu* createPreventedFromClosingMenu(const QString& title, QMenu* menu)
{
    auto* result = new PreventedFromClosingMenu(title, menu);
    if(menu != nullptr) {
        menu->addMenu(result);
    }
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

class LambdaEventFilterObject : public QObject
{
public:
    LambdaEventFilterObject(QObject* parent, const QtEventFilterHandler& handler)
        : QObject(parent)
        , m_handler(handler)
    {
        parent->installEventFilter(this);
    }

    bool eventFilter(QObject *watched, QEvent *event) override { return m_handler(watched, event); }
private:
    QtEventFilterHandler m_handler;
};

void installEventFilter(QObject* target, const QtEventFilterHandler& eventFilter)
{
    new LambdaEventFilterObject(target, eventFilter);
}

#endif


