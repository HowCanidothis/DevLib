#include "WidgetsDialog.h"
#include "ui_widgetsdialog.h"

#include <QPushButton>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDialog::WidgetsDialog(QWidget *parent) :
    Super(parent),
    ui(new Ui::WidgetsDialog),
    m_content(nullptr)
{
    ui->setupUi(this);
    WidgetWrapper(ui->label).WidgetVisibility().ConnectFrom(CDL, [](const QString& t) {
        return !t.isEmpty();
    }, *WidgetLabelWrapper(ui->label).WidgetText());
}

WidgetsDialog::~WidgetsDialog()
{
    delete ui;
}

void WidgetsDialog::Initialize(const std::function<void (qint32)>& onDone, const std::function<void (const QVector<QAbstractButton*>&)>& handler)
{
    for(const auto& button : m_buttons) {
        ui->horizontalLayout->insertWidget(1, button);
    }
    if(handler != nullptr) {
        handler(m_buttons);
    }
    m_onDone = onDone;
}

void WidgetsDialog::SetHeaderText(const FTranslationHandler& text)
{
    WidgetLabelWrapper(ui->label).WidgetText()->SetTranslationHandler(text);
}

void WidgetsDialog::AddButton(const WidgetsDialogsManagerButtonStruct& b)
{
    auto* button = new QPushButton();

    auto setRole = [](QAbstractButton* button, ButtonRole role) {
         WidgetPushButtonWrapper((QPushButton*)button).SetControl(role, true);
    };

    switch(b.Role) {
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
    button->setText(b.Text());
    auto index = m_buttons.size();
    m_buttons.append(button);
    WidgetAbstractButtonWrapper(button).SetOnClicked([this, index]{
        done(index);
    });
}

void WidgetsDialog::SetContent(QWidget* view)
{
    Q_ASSERT(m_content == nullptr);
    view->setSizePolicy(view->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    ui->verticalLayout->insertWidget(1, view);
    m_content = view;
}

void WidgetsDialog::done(int v)
{
    if(m_onDone != nullptr) {
        m_onDone(v);
    }
    Super::done(v);
}
