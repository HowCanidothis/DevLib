#include "shared_decl.h"
#include <QDebug>
#include <QDir>

Q_LOGGING_CATEGORY(LC_UI, "ui")
Q_LOGGING_CATEGORY(LC_SYSTEM, "system")

DirBinder::DirBinder(const QString& dir)
{
    old_path = QDir::currentPath();
    QDir::setCurrent(dir);
}

DirBinder::~DirBinder()
{
    QDir::setCurrent(old_path);
}
