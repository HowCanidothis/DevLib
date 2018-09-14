#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "propertiesdialogbase.h"

class _Export PropertiesDialog : public PropertiesDialogBase
{
    typedef PropertiesDialogBase Super;
public:
    PropertiesDialog(const QString& name, qint32 contextIndex, QWidget* parent = nullptr, Qt::WindowFlags flags= Qt::WindowFlags());

    // PropertiesDialogBase interface
protected:
    virtual void changeProperties(const StdHandle& changingProperties) Q_DECL_OVERRIDE;
};

#endif // PROPERTIESDIALOG_H
