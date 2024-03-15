#include "widgetsheaderlayout.h"
#include "ui_widgetsheaderlayout.h"

#include <WidgetsModule/internal.hpp>

WidgetsHeaderLayout::WidgetsHeaderLayout(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::WidgetsHeaderLayout)
{
    ui->setupUi(this);
    WidgetLineEditWrapper(ui->prefix).SetDynamicSizeAdjusting().WidgetReadOnly() = true;
    WidgetLineEditWrapper(ui->lineEdit).SetDynamicSizeAdjusting();
}

WidgetsHeaderLayout::~WidgetsHeaderLayout()
{
    delete ui;
}

QLineEdit* WidgetsHeaderLayout::prefixEdit() const
{
    return ui->prefix;
}

QLineEdit* WidgetsHeaderLayout::titleEdit() const
{
    return ui->lineEdit;
}

QString WidgetsHeaderLayout::prefix() const
{
    return prefixEdit()->text();
}

QString WidgetsHeaderLayout::title() const
{
    return titleEdit()->text();
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

void WidgetsHeaderLayout::setHasPrefix(bool prefic)
{
    prefixEdit()->setVisible(prefic);
}

void WidgetsHeaderLayout::setReadOnlyPrefix(bool readOnly)
{
    prefixEdit()->setReadOnly(readOnly);
}
