#include "shared_decl.h"
#include <QDebug>
#include <QDir>

Messager::Messager(const char* lbl, const char* file, qint32 line)
    : label(lbl)
{
    QTextStream stream(&location, QIODevice::WriteOnly);
    stream << "   Loc: [" << file << ":" << line << "] " << label << ":";
}

void Messager::error(const char* fmt, ...) const
{
    va_list vl;
    va_start(vl, fmt);
    error() << QString::vasprintf(fmt, vl);
    va_end(vl);
}

void Messager::warning(const char* fmt, ...) const
{
    va_list vl;
    va_start(vl, fmt);
    warning() << QString::vasprintf(fmt, vl);
    va_end(vl);
}

void Messager::info(const char* fmt, ...) const {
    va_list vl;
    va_start(vl, fmt);
    info() << QString::vasprintf(fmt, vl);
    va_end(vl);
}

QDebug Messager::error() const
{
    return qCritical() << location.data();
}

QDebug Messager::warning() const
{
    return qWarning() << location.data();
}

QDebug Messager::info() const
{
    return qInfo() << location.data();
}



DirBinder::DirBinder(const QString& dir)
{
    old_path = QDir::currentPath();
    QDir::setCurrent(dir);
}

DirBinder::~DirBinder()
{
    QDir::setCurrent(old_path);
}
