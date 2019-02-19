#include "qtqssreader.h"

#ifndef NO_GUI

#include <QFile>
#include <QFileInfo>
#include <QApplication>

#include "external.hpp"

QtQSSReader::QtQSSReader()
{

}

QtQSSReader::~QtQSSReader()
{

}

void QtQSSReader::SetEnableObserver(bool enable)
{
    if(enable) {
        _observer = new QtObserver(500);
    } else {
        _observer = nullptr;
    }
}

void QtQSSReader::Install(const QString& mainQSSFile)
{ 
    _fileName = mainQSSFile;

    ((QApplication*)QApplication::instance())->setStyleSheet(ReadAll());
}

QString QtQSSReader::ReadAll()
{        
    if(_observer) {
        _observer->Clear();
        _observer->AddFileObserver(_fileName, [this]{
            Install(_fileName);
        });
    }

    QString result;
    QFileInfo fi(_fileName);
    QFile file(fi.absoluteFilePath());
    if(file.open(QFile::ReadOnly)) {
        QString importsFile = file.readAll();
        QRegExp re("@import url\\(\"([^\\)]*)\"\\);");
        qint32 pos(0);
        while ((pos = re.indexIn(importsFile,pos)) != -1) {
            QString qssFileName = re.cap(1);
            QFile qssFile(fi.absolutePath() + "/" + qssFileName);
            if(qssFile.open(QFile::ReadOnly)) {
                if(_observer) {
                    _observer->AddFileObserver(fi.absolutePath(), qssFileName, [this]{
                        Install(_fileName);
                    });
                }
                result += qssFile.readAll();
            } else {
                qCWarning(LC_SYSTEM) << "No such file" << qssFileName;
            }
            pos += re.matchedLength();
        }
    } else {
        qCWarning(LC_SYSTEM) << file.errorString() << fi.absoluteFilePath();
    }
    return result;
}

#endif
