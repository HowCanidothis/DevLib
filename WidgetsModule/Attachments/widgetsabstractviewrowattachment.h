#ifndef WIDGETSABSTRACTVIEWROWATTACHMENT_H
#define WIDGETSABSTRACTVIEWROWATTACHMENT_H

#include <SharedModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

class Latin1Name;
class WidgetAbstractButtonWrapper;
class QTableView;
class QWidget;

class WidgetsAbstractViewRowAttachment
{
public:
    WidgetsAbstractViewRowAttachment(const std::function<QWidget* (WidgetsAbstractViewRowAttachment*)>& widgetCreator);

    void Attach(QAbstractItemView* v);
    bool SelectCurrentRow();
    void ConnectButton(const Latin1Name& action, const WidgetAbstractButtonWrapper& button, const FTranslationHandler& dialogText = nullptr, const WidgetsDialogsManagerButtonStruct& confirmButton = WidgetsDialogsManagerDefaultButtons::DiscardButton());

    LocalProperty<QModelIndex> CurrentIndex;

private:
    QAbstractItemView* m_target;
    QWidget* m_pane;
};

#endif // WIDGETSABSTRACTVIEWROWATTACHMENT_H
