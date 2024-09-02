#ifndef WIDGETSTABLEVIEWROWATTACHMENT_H
#define WIDGETSTABLEVIEWROWATTACHMENT_H

#include <SharedModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

class Latin1Name;
class WidgetAbstractButtonWrapper;
class QTableView;
class QWidget;

class WidgetsTableViewRowAttachment
{
public:
    WidgetsTableViewRowAttachment(const std::function<QWidget* (WidgetsTableViewRowAttachment*)>& widgetCreator);

    void Attach(QTableView* v);
    bool SelectCurrentRow();
    void ConnectButton(const Latin1Name& action, const WidgetAbstractButtonWrapper& button, const FTranslationHandler& dialogText = nullptr, const WidgetsDialogsManagerButtonStruct& confirmButton = WidgetsDialogsManagerButtonStruct());

    qint32 CurrentRow;

private:
    QTableView* m_target;
    QWidget* m_pane;
};

#endif // WIDGETSTABLEVIEWROWATTACHMENT_H
