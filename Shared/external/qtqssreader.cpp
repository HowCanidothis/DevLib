#include "qtqssreader.h"
#include "Shared/shared_decl.h"
#include <QFile>
#include <QFileInfo>
#include <QApplication>

#include "external.hpp"

QtQSSReader::QtQSSReader(const QString& main_qss_file)
    : file_name(main_qss_file)
{

}

void QtQSSReader::install(const QString* main_qss_file)
{
    QtQSSReader reader(*main_qss_file);
    ((QApplication*)QApplication::instance())->setStyleSheet(reader.readAll());
}

void QtQSSReader::installAndObserve(const QString* main_qss_file)
{
    static bool installed = false;
    Q_ASSERT(installed == false);

    Observer::instance()->addFileObserver(main_qss_file, [main_qss_file]() {
        install(main_qss_file);
    }
    );
    install(main_qss_file);
}

QString QtQSSReader::readAll() const
{
    QString result;
    QFileInfo fi(file_name);
    DirBinder dir(fi.absolutePath());
    QFile file(fi.fileName());
    if(file.open(QFile::ReadOnly)) {
        QString imports_file = file.readAll();
        QRegExp re("@import url\\(\"([^\\)]*)\"\\);");
        qint32 pos(0);
        while ((pos = re.indexIn(imports_file,pos)) != -1) {
            QString qss_file_name = re.cap(1);
            QFile qss_file(qss_file_name);
            if(qss_file.open(QFile::ReadOnly)) {
                result += qss_file.readAll();
            }
            pos += re.matchedLength();
        }
    }
    return result;
}
