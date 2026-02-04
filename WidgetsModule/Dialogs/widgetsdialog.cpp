#include "widgetsdialog.h"
#include "ui_widgetsdialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QKeyEvent>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDialog::WidgetsDialog(QWidget *parent) :
    Super(parent),
    ui(new Ui::WidgetsDialog),
    m_content(nullptr)
{
    ui->setupUi(this);
    WidgetWrapper(ui->widget_2).WidgetVisibility().ConnectFrom(CDL, [](const QString& t) {
        return !t.isEmpty();
    }, *WidgetLabelWrapper(ui->label).WidgetText());

    setAttribute(Qt::WA_OpaquePaintEvent);
}

WidgetsDialog::~WidgetsDialog()
{
    delete ui;
}

void WidgetsDialog::Initialize(const std::function<void (qint32)>& onDone, const std::function<void (const QVector<QAbstractButton*>&)>& handler, const std::function<void (bool)>& dontShowHandler)
{    
    ui->widget->setVisible(m_buttons.size() > 0);

    auto sortedByRole = m_buttons;
    /*Cancel/Discard/Save/Action*/
    static auto priorityMap = Factory<qint32, qint32>().Insert((qint32)ButtonRole::Cancel, 0)
                                                           .Insert((qint32)ButtonRole::Reset, 1)
                                                           .Insert((qint32)ButtonRole::Save, 2);
    std::sort(sortedByRole.begin(), sortedByRole.end(), [this](auto* f, auto* s) {
        auto frole = (qint32)m_roles.value(f, ButtonRole::Save);
        auto srole = (qint32)m_roles.value(s, ButtonRole::Save);
        return priorityMap.value(frole, 3) < priorityMap.value(srole, 3);
    });
    for(auto* button : sortedByRole) {
        auto bRole = m_roles.value(button, ButtonRole::Save);
        switch(bRole) {
        case ButtonRole::IconWithText: ui->horizontalLayout->insertWidget(0, button);
            break;
        default: ui->horizontalLayout->addWidget(button);
            break;
        }
    }
    if(handler != nullptr) {
        handler(m_buttons);
    }
    m_onDone = onDone;
    if(dontShowHandler != nullptr) {
        auto* checkBox = new QCheckBox(tr("Do not show this message again"));
        WidgetCheckBoxWrapper(checkBox).WidgetChecked().Connect(CDL, dontShowHandler);
        if(m_content == nullptr) {
            ui->verticalLayout->insertWidget(1, checkBox);
        } else {
            ui->verticalLayout->insertWidget(2, checkBox);
        }
    }
    m_roles.clear();
}

void WidgetsDialog::SetHeaderText(const FTranslationHandler& text)
{
    WidgetLabelWrapper(ui->label).WidgetText()->SetTranslationHandler(text);
}

QAbstractButton* WidgetsDialog::AddButton(const WidgetsDialogsManagerButtonStruct& b)
{
    auto* button = new QPushButton();

    auto setRole = [this](QAbstractButton* button, ButtonRole role) {
         WidgetPushButtonWrapper((QPushButton*)button).SetControl(role, true);
         m_roles.insert(button, role);
    };

    switch(b.Role) {
    case QDialogButtonBox::ActionRole:
        if(!b.Icon.IsNull()) {
            WidgetAbstractButtonWrapper(button).SetIcon(b.Icon);
        }
        setRole(button, ButtonRole::IconWithText); break;
    case QDialogButtonBox::YesRole:
    case QDialogButtonBox::ApplyRole:
    case QDialogButtonBox::AcceptRole: setRole(button, ButtonRole::Save); break;
    case QDialogButtonBox::NoRole:
    case QDialogButtonBox::ResetRole:
    case QDialogButtonBox::RejectRole: setRole(button, ButtonRole::Cancel); break;
    case QDialogButtonBox::DestructiveRole: setRole(button, ButtonRole::Reset); break;
    default: Q_ASSERT(false); break;
    }
    if(b.Text != nullptr) {
        button->setText(b.Text());
    }
    auto index = m_buttons.size();
    m_buttons.append(button);
    if(b.Role == QDialogButtonBox::ActionRole) {
        WidgetAbstractButtonWrapper(button).SetOnClicked([this, index]{
            m_onDone(index);
        });
    } else {
        WidgetAbstractButtonWrapper(button).SetOnClicked([this, index]{
            done(index);
        });
    }
    button->setDefault(true);
    return button;
}

void WidgetsDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Cancel)) {
        done(0);
    } else if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            for (auto* btn : m_buttons) {
                auto pb = qobject_cast<QPushButton*>(btn);
                if (pb->isDefault() && pb->isVisible()) {
                    if (pb->isEnabled())
                        pb->click();
                    return;
                }
            }
        }
        break;
        default:
            e->ignore();
            return;
        }
    } else {
        e->ignore();
    }
}

void WidgetsDialog::paintEvent(QPaintEvent* e)
{
    Super::paintEvent(e);
}

void WidgetsDialog::SetContent(QWidget* view)
{
    Q_ASSERT(m_content == nullptr);
    ui->verticalLayout->insertWidget(1, view);
    m_content = view;
    WidgetWrapper(m_content).AddEventFilter([this](QObject*, QEvent* e){
        if(e->type() == QEvent::KeyPress) {
            event(e);
            if(e->isAccepted()) {
                return true;
            }
        }
        return false;
    });
}

void WidgetsDialog::done(int v)
{
    if(m_onDone != nullptr) {
        m_onDone(v);
    }
    Super::done(v);
}
