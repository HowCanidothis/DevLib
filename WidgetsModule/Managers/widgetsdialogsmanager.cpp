#include "widgetsdialogsmanager.h"

WidgetsDialogsManager::WidgetsDialogsManager()
{

}

WidgetsDialogsManager& WidgetsDialogsManager::GetInstance()
{
    static WidgetsDialogsManager result;
    return result;
}
