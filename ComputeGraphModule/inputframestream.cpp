#include "inputframestream.h"

#include <opencv2/opencv.hpp>
#include <QFile>
#include <QDir>
#include <QDataStream>

InputFrameStream::InputFrameStream(quint32 outputs_count)
    : output(new cv::Mat)
    , input_file(new QFile())
    , input_stream(new QDataStream(input_file.data()))
    , outputs_count(outputs_count)
    , reading_counter(outputs_count)
    , paused(false)
{
    input_file->open(QFile::ReadOnly);
}

InputFrameStream::~InputFrameStream()
{

}

void InputFrameStream::moveToThread(QThread* thread)
{
    input_file->moveToThread(thread);
}

void InputFrameStream::setFileName(const QString& name)
{
    LOGOUT;
    if(input_file->isOpen())
    {
        if(input_file->fileName() == name)
            return;
        input_file->close();
    }
    input_file->setFileName(name);
    if(!input_file->open(QFile::ReadOnly))
        log.Warning() << input_file->errorString();
    input_stream->device()->seek(0);
}

bool InputFrameStream::readFrame()
{
    if(paused) return true;
    if(reading_counter-- == outputs_count) {
        if(!input_stream->atEnd()) {
            qint32 cv_type;
            cv::Size size;
            input_stream->readRawData((char*)&cv_type, sizeof(qint32));
            input_stream->readRawData((char*)&size, sizeof(cv::Size));

            output->create(size, cv_type);
            input_stream->readRawData((char*)output->data, qint32(output->elemSize() * output->total()));
        }
    }
    if(!reading_counter) {
        if(input_stream->atEnd())
            repeat();
        reading_counter = outputs_count;
    }
    return true;
}

void InputFrameStream::repeat()
{
    input_stream->device()->seek(0);
}

bool InputFrameStream::isFinished() const
{
    return input_stream->atEnd();
}

bool InputFrameStream::isValid() const
{
    return input_stream->device()->size();
}

const cv::Mat& InputFrameStream::getOutput() const
{
    return *output.data();
}

QStringList InputFrameStream::getAvailableInputs()
{
    QDir current;
    QStringList res = current.entryList({"*.dvx"}, QDir::Files);
    return res;
}
