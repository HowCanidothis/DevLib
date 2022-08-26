#include "propertiesdialog.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QDialogButtonBox>

#include "propertiesmodel.h"
#include "propertiesview.h"

PropertiesDialog::PropertiesDialog(const PropertiesScopeName& contextIndex, QWidget* parent)
    : Super(contextIndex, new PropertiesView(contextIndex), parent)
{
}

void PropertiesDialog::changeProperties(const StdHandle& changingProperties)
{
    reinterpret_cast<PropertiesView*>(m_view)->GetPropertiesModel()->Change(changingProperties);
}

void PropertiesDialog::done(int result)
{
    reinterpret_cast<PropertiesView*>(m_view)->setCurrentIndex(QModelIndex());
    Super::done(result);
}
