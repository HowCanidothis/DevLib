#include "qtqssreader.h"
#include "SharedModule/decl.h"
#include <QFile>
#include <QFileInfo>
#include <QApplication>

#include "external.hpp"

QtQSSReader::QtQSSReader(const QString& mainQSSFile)
    : _fileName(mainQSSFile)
{

}

void QtQSSReader::Install(const QString* mainQSSFile)
{
    QtQSSReader reader(*mainQSSFile);
    ((QApplication*)QApplication::instance())->setStyleSheet(reader.ReadAll());
}

void QtQSSReader::InstallAndObserve(const QString* mainQSSFile)
{
    static bool installed = false;
    Q_ASSERT(installed == false);

    Observer::Instance()->AddFileObserver(mainQSSFile, [mainQSSFile]() {
        Install(mainQSSFile);
    });
    Install(mainQSSFile);
}

QString QtQSSReader::ReadAll() const
{
    QString result;
    QFileInfo fi(_fileName);
    DirBinder dir(fi.absolutePath());
    QFile file(fi.fileName());
    if(file.open(QFile::ReadOnly)) {
        QString importsFile = file.readAll();
        QRegExp re("@import url\\(\"([^\\)]*)\"\\);");
        qint32 pos(0);
        while ((pos = re.indexIn(importsFile,pos)) != -1) {
            QString qssFileName = re.cap(1);
            QFile qssFile(qssFileName);
            if(qssFile.open(QFile::ReadOnly)) {
                result += qssFile.readAll();
            }
            pos += re.matchedLength();
        }
    }
    return result;
}
