#include "qtqssreader.h"

#include <QFile>
#include <QFileInfo>

#include <QApplication>
#include <QRegularExpression>

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

    QByteArray result;
    QFileInfo fi(_fileName);
    QFile file(fi.absoluteFilePath());
    if(file.open(QFile::ReadOnly)) {
        QByteArray importsFile = file.readAll();
        thread_local static QRegularExpression rePal(R"((@c[\d\w_\.]+):([^;]+))");
        thread_local static QRegularExpression reResources(R"(@resource[\d\w_\.]*:([^;]+))");
        thread_local static QRegularExpression re("@import url\\(\"([^\\)]*)\"\\);");
        thread_local static QRegularExpression reColors(R"((@c[\d\w_\.]+))");
        QHash<Name, QString> colors;
        QRegularExpressionMatchIterator it;
        importsFile.replace("@temp", SharedSettings::GetInstance().PathSettings.TempDir.absolutePath().toUtf8());
        it = rePal.globalMatch(importsFile, 0);
        while(it.hasNext()) {
            auto next = it.next();
            colors.insert(Name(next.captured(1)), next.captured(2));
        }
        auto applyConstants = [&colors](QByteArray& result, const QByteArray& fileContent) {
            auto colorsIt = reColors.globalMatch(fileContent);
            qint32 prev = 0;
            while(colorsIt.hasNext()) {
                auto match = colorsIt.next();
                for(const auto& sym : adapters::range(fileContent, prev, match.capturedStart() - prev)) {
                    result += sym;
                }
                result += colors.value(Name(match.captured(1)), "#00000000");
                prev = match.capturedEnd();
            }
            for(const auto& sym : adapters::range(fileContent, prev, fileContent.size() - prev)) {
                result += sym;
            }
            return result;
        };
        it = reResources.globalMatch(importsFile, 0);
        while(it.hasNext()) {
            QDir dir(it.next().captured(1));
            for(const QString& s : dir.entryList()) {
                auto newFileName = SharedSettings::GetInstance().PathSettings.TempDir.absoluteFilePath(s);
                QFile templateFile(dir.absoluteFilePath(s));
                QFile newFile(newFileName);
                if(templateFile.open(QFile::ReadOnly) && newFile.open(QFile::WriteOnly)) {
                    auto fileContent = templateFile.readAll();
                    QByteArray newFileContent;
                    applyConstants(newFileContent, fileContent);
                    newFile.write(newFileContent);
                }
            }
        }
        it = re.globalMatch(importsFile, 0);
        while (it.hasNext()) {
            QString qssFileName = it.next().captured(1);
            QFile qssFile(fi.absolutePath() + "/" + qssFileName);
            if(qssFile.open(QFile::ReadOnly)) {
                if(_observer) {
                    _observer->AddFileObserver(fi.absolutePath(), qssFileName, [this]{
                        Install(_fileName);
                    });
                }
                auto fileContent = qssFile.readAll();
                applyConstants(result, fileContent);
            } else {
                qCWarning(LC_SYSTEM) << "No such file" << qssFileName;
            }
        }
    } else {
        qCWarning(LC_SYSTEM) << file.errorString() << fi.absoluteFilePath();
    }
    QFile test("C:/Work/temp/test.txt");
    if(test.open(QFile::WriteOnly)) {
        test.write(result);
    }
    return result;
}
