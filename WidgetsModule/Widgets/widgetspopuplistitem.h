#ifndef WIDGETSPOPUPLISTITEM_H
#define WIDGETSPOPUPLISTITEM_H

#include <PropertiesModule/Models/wrappers.h>

// Строка данных WidgetsGroupedPopupList. Группы строятся по Group в порядке первого появления в модели.
struct WidgetsPopupListItem
{
    Name Id;
    QString Title;
    QString Group;
    bool Enabled = true;
    bool Favorite = false;
    bool Pinned = false;

    bool operator==(const WidgetsPopupListItem& other) const
    {
        return Id == other.Id && Title == other.Title && Group == other.Group
            && Enabled == other.Enabled && Favorite == other.Favorite && Pinned == other.Pinned;
    }
    bool operator!=(const WidgetsPopupListItem& other) const { return !(*this == other); }
};
DECLARE_MODEL_BY_TYPE(WidgetsPopupListItem)

#endif // WIDGETSPOPUPLISTITEM_H
