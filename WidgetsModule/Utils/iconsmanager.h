#ifndef ICONSMANAGER_H
#define ICONSMANAGER_H

#include <QIcon>

class IconsManager
{
public:
    IconsManager(qint32 iconsCount);

    void RegisterIcon(qint32 index, const QString& path);
    const QIcon& GetIcon(qint32 index) const;

    static IconsManager& GetInstance();

private:
    QVector<QIcon> m_icons;
    static IconsManager* m_instance;
};

#endif // ICONSMANAGER_H
