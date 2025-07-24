#include "widgetsheaderlayout.h"
#include "ui_widgetsheaderlayout.h"

#include <QPushButton>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsHeaderLayout::WidgetsHeaderLayout(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsHeaderLayout)
{
    ui->setupUi(this);
    WidgetLineEditWrapper prefixWrapper(ui->prefix);
    WidgetLineEditWrapper valueWrapper(ui->value);
    prefixWrapper.SetDynamicSizeAdjusting().WidgetReadOnly() = true;
    valueWrapper.SetDynamicSizeAdjusting();
    setFocusProxy(ui->value);
    prefixWrapper.WidgetVisibility() = true;
    valueWrapper.WidgetVisibility() = true;
    WidgetWrapper(ui->separator).WidgetVisibility().ConnectFrom(CDL, [](bool v1, bool v2) {
        return v1 && v2;
    }, prefixWrapper.WidgetVisibility(), valueWrapper.WidgetVisibility());
}

WidgetsHeaderLayout::~WidgetsHeaderLayout()
{
    delete ui;
}

QLabel* WidgetsHeaderLayout::titleEdit() const
{
    return ui->title;
}

QLineEdit* WidgetsHeaderLayout::prefixEdit() const
{
    return ui->prefix;
}

QPushButton* WidgetsHeaderLayout::lockButton() const
{
    if(m_lockButton == nullptr) {
        return nullptr;
    }
    return m_lockButton->Widget;
}

QLineEdit* WidgetsHeaderLayout::valueEdit() const
{
    return ui->value;
}

bool WidgetsHeaderLayout::hasLockButton() const
{
    return m_lockButton != nullptr;
}

void WidgetsHeaderLayout::setHasLockButton(bool enabled)
{
    if(enabled) {
        if(m_lockButton != nullptr) {
            return;
        }
        m_lockButton = new WidgetsLayoutComponent<QPushButton>("BtnLocked");
        ui->horizontalLayout_2->insertWidget(0, m_lockButton->Widget);
    } else if(m_lockButton != nullptr){
        m_lockButton->Detach();
        m_lockButton = nullptr;
    }
}

QString WidgetsHeaderLayout::title() const
{
    return titleEdit()->text();
}

QString WidgetsHeaderLayout::prefix() const
{
    return prefixEdit()->text();
}

QString WidgetsHeaderLayout::value() const
{
    return valueEdit()->text();
}

bool WidgetsHeaderLayout::hasValue() const
{
    return valueEdit()->isVisibleTo(this);
}

bool WidgetsHeaderLayout::hasPrefix() const
{
    return prefixEdit()->isVisibleTo(this);
}

bool WidgetsHeaderLayout::readOnlyPrefix() const
{
    return prefixEdit()->isReadOnly();
}

void WidgetsHeaderLayout::setTitle(const QString& title)
{
    titleEdit()->setText(title);
}

void WidgetsHeaderLayout::setPrefix(const QString& prefix)
{
    prefixEdit()->setText(prefix);
}

void WidgetsHeaderLayout::setValue(const QString& value)
{
    valueEdit()->setText(value);
}


void WidgetsHeaderLayout::setHasValue(bool value)
{
    WidgetWrapper(valueEdit()).SetVisible(value);
}

void WidgetsHeaderLayout::setHasPrefix(bool prefic)
{
    WidgetWrapper(prefixEdit()).SetVisible(prefic);
}

void WidgetsHeaderLayout::setReadOnlyPrefix(bool readOnly)
{
    prefixEdit()->setReadOnly(readOnly);
}
