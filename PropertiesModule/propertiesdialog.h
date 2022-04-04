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
    PropertiesDialog(const PropertiesScopeName& contextIndex, QWidget* parent = nullptr);

    // PropertiesDialogBase interface
protected:
    void changeProperties(const StdHandle& changingProperties) override;
    void done(int result) override;
};

#endif // QT_GUI_LIB

#endif // PROPERTIESDIALOG_H
