#include "propertiesdialog.h"

#ifdef QT_GUI_LIB

#include <QVBoxLayout>
#include <QApplication>
#include <QDialogButtonBox>

#include "propertiesmodel.h"
#include "propertiessystem.h"
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

#endif
