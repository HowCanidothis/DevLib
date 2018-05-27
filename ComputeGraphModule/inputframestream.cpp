#include "inputframestream.h"

#include <opencv2/opencv.hpp>
#include <QFile>
#include <QDir>
#include <QDataStream>

InputFrameStream::InputFrameStream(quint32 outputsCount)
    : _output(new cv::Mat)
    , _inputFile(new QFile())
    , _inputStream(new QDataStream(_inputFile.data()))
    , _outputsCount(outputsCount)
    , _readingCounter(outputsCount)
    , _paused(false)
{
    _inputFile->open(QFile::ReadOnly);
}

InputFrameStream::~InputFrameStream()
{

}

void InputFrameStream::MoveToThread(QThread* thread)
{
    _inputFile->moveToThread(thread);
}

void InputFrameStream::SetFileName(const QString& name)
{
    LOGOUT;
    if(_inputFile->isOpen())
    {
        if(_inputFile->fileName() == name)
            return;
        _inputFile->close();
    }
    _inputFile->setFileName(name);
    if(!_inputFile->open(QFile::ReadOnly))
        log.Warning() << _inputFile->errorString();
    _inputStream->device()->seek(0);
}

bool InputFrameStream::ReadFrame()
{
    if(_paused) {
        return true;
    }
    if(_readingCounter-- == _outputsCount) {
        if(!_inputStream->atEnd()) {
            qint32 cvType;
            cv::Size size;
            _inputStream->readRawData((char*)&cvType, sizeof(qint32));
            _inputStream->readRawData((char*)&size, sizeof(cv::Size));

            _output->create(size, cvType);
            _inputStream->readRawData((char*)_output->data, qint32(_output->elemSize() * _output->total()));
        }
    }
    if(!_readingCounter) {
        if(_inputStream->atEnd())
            Repeat();
        _readingCounter = _outputsCount;
    }
    return true;
}

void InputFrameStream::Repeat()
{
    _inputStream->device()->seek(0);
}

bool InputFrameStream::IsFinished() const
{
    return _inputStream->atEnd();
}

bool InputFrameStream::IsValid() const
{
    return _inputStream->device()->size();
}

const cv::Mat& InputFrameStream::GetOutput() const
{
    return *_output.data();
}

QStringList InputFrameStream::GetAvailableInputs()
{
    QDir current;
    QStringList res = current.entryList({"*.dvx"}, QDir::Files);
    return res;
}
