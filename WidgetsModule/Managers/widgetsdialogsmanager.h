#ifndef WIDGETSDIALOGSMANAGER_H
#define WIDGETSDIALOGSMANAGER_H


class WidgetsDialogsManager
{
    WidgetsDialogsManager();
public:
    static WidgetsDialogsManager& GetInstance();

    CommonDispatcher<QWidget*> OnDialogCreated;
};

#endif // WIDGETSDIALOGMANAGER_H
