#include "decl.h"
#include <QDebug>
#include <QDir>

Messager::Messager(const char* lbl, const char* file, qint32 line)
    : _label(lbl)
{
    QTextStream stream(&_location, QIODevice::WriteOnly);
    stream << "   Loc: [" << file << ":" << line << "] " << _label << ":";
}

void Messager::Error(const char* fmt, ...) const
{
    va_list vl;
    va_start(vl, fmt);
    Error() << QString::vasprintf(fmt, vl);
    va_end(vl);
}

void Messager::Warning(const char* fmt, ...) const
{
    va_list vl;
    va_start(vl, fmt);
    Warning() << QString::vasprintf(fmt, vl);
    va_end(vl);
}

void Messager::Info(const char* fmt, ...) const {
    va_list vl;
    va_start(vl, fmt);
    Info() << QString::vasprintf(fmt, vl);
    va_end(vl);
}

QDebug Messager::Error() const
{
    return qCritical() << _location.data();
}

QDebug Messager::Warning() const
{
    return qWarning() << _location.data();
}

QDebug Messager::Info() const
{
    return qInfo() << _location.data();
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
