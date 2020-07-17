#include "iconsmanager.h"

IconsManager* IconsManager::m_instance = nullptr;

IconsManager::IconsManager(qint32 iconsCount)
    : m_icons(iconsCount)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;
}

void IconsManager::RegisterIcon(qint32 index, const QString& path)
{
    m_icons[index] = QIcon(path);
}

const QIcon& IconsManager::GetIcon(qint32 index) const
{
    return m_icons.at(index);
}

IconsManager& IconsManager::GetInstance()
{
    return *m_instance;
}
