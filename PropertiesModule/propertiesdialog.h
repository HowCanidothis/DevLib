#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "propertiesdialogbase.h"

/*
 * PropertiesDialog uses default view model for visualizing properties PropertiesView and PropertiesModel.
 * Note: Deleting properties dialogs is restricted
*/

#ifdef QT_GUI_LIB

class _Export PropertiesDialog : public PropertiesDialogBase
{
    typedef PropertiesDialogBase Super;
public:
    PropertiesDialog(const QString& name, const PropertiesScopeName& contextIndex, QWidget* parent = nullptr);

    // PropertiesDialogBase interface
protected:
    virtual void changeProperties(const StdHandle& changingProperties) Q_DECL_OVERRIDE;
};

#endif // QT_GUI_LIB

#endif // PROPERTIESDIALOG_H
