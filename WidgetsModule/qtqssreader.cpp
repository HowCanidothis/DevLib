#include "qtqssreader.h"

#include <QFile>
#include <QFileInfo>

#include <QApplication>

#include <SharedModule/External/external.hpp>

QtQSSReader::QtQSSReader()
{

}

QtQSSReader::~QtQSSReader()
{

}

void QtQSSReader::SetEnableObserver(bool enable)
{
    if(enable) {
        _observer = new QtObserver(500, ThreadHandlerNoCheckMainLowPriority);
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
            ThreadsBase::DoMain(CONNECTION_DEBUG_LOCATION,[this]{
                Install(_fileName);
            });
        });
    }

    QString result;
    QFileInfo fi(_fileName);
    QFile file(fi.absoluteFilePath());
    if(file.open(QFile::ReadOnly)) {
        QString importsFile = file.readAll();
        thread_local static QRegExp rePal(R"((@c[\d\w_\.]+):([^;]+))");
        thread_local static QRegExp re("@import url\\(\"([^\\)]*)\"\\);");
        qint32 pos(0);
        QHash<Name, QString> colors;
        while ((pos = rePal.indexIn(importsFile,pos)) != -1) {
            colors.insert(Name(rePal.cap(1)), rePal.cap(2));
            pos += rePal.matchedLength();
        }
        pos = 0;
        while ((pos = re.indexIn(importsFile,pos)) != -1) {
            QString qssFileName = re.cap(1);
            QFile qssFile(fi.absolutePath() + "/" + qssFileName);
            if(qssFile.open(QFile::ReadOnly)) {
                if(_observer) {
                    _observer->AddFileObserver(fi.absolutePath(), qssFileName, [this]{
                        Install(_fileName);
                    });
                }
                thread_local static QRegExp reColors(R"((@c[\d\w_\.]+))");
                auto fileContent = qssFile.readAll();
                qint32 current = 0, prev = 0;
                while((current = reColors.indexIn(fileContent,current)) != -1) {
                    for(const auto& sym : adapters::range(fileContent, prev, current - prev)) {
                        result += sym;
                    }
                    result += colors.value(Name(reColors.cap(1)), "#00000000");
                    current += reColors.matchedLength();
                    prev = current;
                }
                for(const auto& sym : adapters::range(fileContent, prev, fileContent.size() - prev)) {
                    result += sym;
                }
            } else {
                qCWarning(LC_SYSTEM) << "No such file" << qssFileName;
            }
            pos += re.matchedLength();
        }
    } else {
        qCWarning(LC_SYSTEM) << file.errorString() << fi.absoluteFilePath();
    }
    QFile test("C:/Work/temp/test.txt");
    if(test.open(QFile::WriteOnly)) {
        test.write(result.toUtf8());
    }
    return result;
}
