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
        NormalColor.ConnectFrom(another.NormalColor);
        DisabledColor.ConnectFrom(another.DisabledColor);
        ActiveColor.ConnectFrom(another.ActiveColor);
        SelectedColor.ConnectFrom(another.SelectedColor);
    }
};

class IconsSvgIcon : public QIcon
{
    using Super = QIcon;
public:
    IconsSvgIcon();
    IconsSvgIcon(const QString& filePath);

    IconsPalette& EditPalette();

private:
    class SvgIconEngine* m_engine;
};

class IconsManager
{
public:
    IconsManager(qint32 iconsCount);

    IconsSvgIcon RegisterIcon(qint32 index, const QString& path);
    const QIcon& GetIcon(qint32 index) const;

    static IconsManager& GetInstance();

private:
    QVector<IconsSvgIcon> m_icons;
    static IconsManager* m_instance;
};

#endif // ICONSMANAGER_H
