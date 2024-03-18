#include "widgetsheaderlayout.h"
#include "ui_widgetsheaderlayout.h"

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/widgetstyleutils.h"

WidgetsHeaderLayout::WidgetsHeaderLayout(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsHeaderLayout)
{
    ui->setupUi(this);
    WidgetLineEditWrapper(ui->prefix).SetDynamicSizeAdjusting().WidgetReadOnly() = true;
    WidgetLineEditWrapper(ui->value).SetDynamicSizeAdjusting();
    setFocusProxy(ui->value);
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

QLineEdit* WidgetsHeaderLayout::valueEdit() const
{
    return ui->value;
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
    return valueEdit()->isVisible();
}

bool WidgetsHeaderLayout::hasPrefix() const
{
    return prefixEdit()->isVisible();
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
    valueEdit()->setVisible(value);
}

void WidgetsHeaderLayout::setHasPrefix(bool prefic)
{
    prefixEdit()->setVisible(prefic);
}

void WidgetsHeaderLayout::setReadOnlyPrefix(bool readOnly)
{
    prefixEdit()->setReadOnly(readOnly);
}
