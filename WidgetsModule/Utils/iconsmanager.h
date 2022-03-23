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

    IconsPalette& SetColors(const QColor& color)
    {
        NormalColor = color;
        DisabledColor = color;
        ActiveColor = color;
        SelectedColor = color;
        return *this;
    }

    IconsPalette& SetSelectedColor(const QColor& color)
    {
        SelectedColor = color;
        return *this;
    }

    IconsPalette& SetActiveColor(const QColor& color)
    {
        ActiveColor = color;
        return *this;
    }

    DispatcherConnections ConnectFrom(const char* location, IconsPalette& another)
    {
        DispatcherConnections result;
        result += NormalColor.ConnectFrom(location, another.NormalColor);
        result += DisabledColor.ConnectFrom(location, another.DisabledColor);
        result += ActiveColor.ConnectFrom(location, another.ActiveColor);
        result += SelectedColor.ConnectFrom(location, another.SelectedColor);
        return result;
    }

    DispatcherConnections ConnectFrom(const char* location, const LocalPropertyColor& color)
    {
        DispatcherConnections result;
        result += NormalColor.ConnectFrom(location, color);
        result += DisabledColor.ConnectFrom(location, color);
        result += ActiveColor.ConnectFrom(location, color);
        result += SelectedColor.ConnectFrom(location, color);
        return result;
    }

    DispatcherConnections ConnectFrom(const char* location, const LocalPropertyColor& color, const LocalPropertyColor& checked)
    {
        DispatcherConnections result;
        result += NormalColor.ConnectFrom(location, color);
        result += DisabledColor.ConnectFrom(location, color);
        result += ActiveColor.ConnectFrom(location, color);
        result += SelectedColor.ConnectFrom(location, checked);
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
    IconsSvgIcon RegisterIconWithDefaultColorScheme(const Name& id, const QString& path);
    const IconsSvgIcon& GetIcon(qint32 index) const;
    const IconsSvgIcon& GetIcon(const Name& id) const;

    static IconsManager& GetInstance();

private:
    QVector<IconsSvgIcon> m_icons;
    QHash<Name, IconsSvgIcon> m_taggedIcons;
    static IconsManager* m_instance;
};

#endif // ICONSMANAGER_H
