#include "WidgetsDialogView.h"
#include "ui_widgetsdialogview.h"

#include <QPushButton>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDialogView::WidgetsDialogView(QWidget *parent) :
    Super(parent),
    ui(new Ui::WidgetsDialogView),
    m_buttonsCount(0)
{
    ui->setupUi(this);
}

WidgetsDialogView::~WidgetsDialogView()
{
    delete ui;
}

void WidgetsDialogView::AddAction(QDialogButtonBox::ButtonRole role, const QString& text)
{
    auto* button = new QPushButton();

    auto setRole = [](QAbstractButton* button, ButtonRole role) {
         WidgetPushButtonWrapper((QPushButton*)button).SetControl(role, true);
    };

    switch(role) {
    case QDialogButtonBox::YesRole:
    case QDialogButtonBox::ApplyRole:
    case QDialogButtonBox::AcceptRole: setRole(button, ButtonRole::Save); break;
    case QDialogButtonBox::ResetRole:
    case QDialogButtonBox::NoRole:
    case QDialogButtonBox::RejectRole: setRole(button, ButtonRole::Cancel); break;
    case QDialogButtonBox::ActionRole: break;
    case QDialogButtonBox::DestructiveRole: setRole(button, ButtonRole::Reset); break;
    default: Q_ASSERT(false); break;
    }
    button->setText(text);
    auto index = m_buttonsCount++;
    WidgetAbstractButtonWrapper(button).SetOnClicked([this, index]{
        done(index);
    });
    ui->horizontalLayout->insertWidget(1, button);
}

void WidgetsDialogView::SetContent(QWidget* view)
{
    delete ui->label;
    view->setSizePolicy(view->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    ui->verticalLayout->insertWidget(0, view);
}
