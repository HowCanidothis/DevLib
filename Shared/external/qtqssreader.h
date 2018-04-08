#ifndef QTQSSREADER_H
#define QTQSSREADER_H

#include <QString>

class QtQSSReader
{
    QString file_name;
public:
    QtQSSReader(const QString& main_qss_file);

    static void install(const QString* main_qss_file);
    static void installAndObserve(const QString* main_qss_file);

    QString readAll() const;
};

#endif // QTQSSREADER_H
