#include "propertiesdialog.h"

#ifdef QT_GUI_LIB

#include <QVBoxLayout>
#include <QApplication>
#include <QDialogButtonBox>

#include "propertiesmodel.h"
#include "propertiessystem.h"
#include "propertiesview.h"

PropertiesDialog::PropertiesDialog(const QString& name, qint32 contextIndex, QWidget* parent)
    : Super(name, contextIndex, new PropertiesView(contextIndex), parent)
{
}

void PropertiesDialog::changeProperties(const StdHandle& changingProperties)
{
    reinterpret_cast<PropertiesView*>(m_view)->GetPropertiesModel()->Change(changingProperties);
}

#endif
