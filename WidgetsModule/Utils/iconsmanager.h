#ifndef ICONSMANAGER_H
#define ICONSMANAGER_H

#include <QIcon>

#include <PropertiesModule/internal.hpp>

struct IconsPalette
{
    LocalPropertyColor NormalColor;
    LocalPropertyColor DisabledColor;
    LocalPropertyColor ActiveColor;
    LocalPropertyColor SelectedColor;

    DispatcherConnections ConnectFrom(IconsPalette& another)
    {
        DispatcherConnections result;
        result += NormalColor.ConnectFrom(another.NormalColor);
        result += DisabledColor.ConnectFrom(another.DisabledColor);
        result += ActiveColor.ConnectFrom(another.ActiveColor);
        result += SelectedColor.ConnectFrom(another.SelectedColor);
        return result;
    }

    DispatcherConnections ConnectFrom(const LocalPropertyColor& color)
    {
        DispatcherConnections result;
        result += NormalColor.ConnectFrom(color);
        result += DisabledColor.ConnectFrom(color);
        result += ActiveColor.ConnectFrom(color);
        result += SelectedColor.ConnectFrom(color);
        return result;
    }

    DispatcherConnections ConnectFrom(const LocalPropertyColor& color, const LocalPropertyColor& checked)
    {
        DispatcherConnections result;
        result += NormalColor.ConnectFrom(color);
        result += DisabledColor.ConnectFrom(color);
        result += ActiveColor.ConnectFrom(color);
        result += SelectedColor.ConnectFrom(checked);
        return result;
    }
};

class IconsSvgIcon : public QIcon
{
    using Super = QIcon;
public:
    IconsSvgIcon();
    IconsSvgIcon(const QString& filePath);

    IconsPalette& EditPalette() const;

private:
    class SvgIconEngine* m_engine;
};

class IconsManager
{
public:
    IconsManager(qint32 iconsCount);

    IconsSvgIcon RegisterIcon(qint32 index, const QString& path);
    IconsSvgIcon RegisterIcon(const Name& id, const QString& path);
    const IconsSvgIcon& GetIcon(qint32 index) const;
    const IconsSvgIcon& GetIcon(const Name& id) const;

    static IconsManager& GetInstance();

private:
    QVector<IconsSvgIcon> m_icons;
    QHash<Name, IconsSvgIcon> m_taggedIcons;
    static IconsManager* m_instance;
};

#endif // ICONSMANAGER_H
