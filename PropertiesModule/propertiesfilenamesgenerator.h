#ifndef PROPERTIESFILENAMESGENERATOR_H
#define PROPERTIESFILENAMESGENERATOR_H

#include <SharedModule/internal.hpp>

#include "property.h"

class PropertiesFileNamesGenerator : public FileNamesGeneratorWithGuard
{
    using Super = FileNamesGeneratorWithGuard;
public:
    PropertiesFileNamesGenerator(const QString& baseName, const QString& format, qint32 maxFilesCount);

private:
    IntProperty m_maxCount;
};

class PropertiesFileNamesGeneratorFileStream
{
    using Super = QStreamBufferWrite;
public:
    PropertiesFileNamesGeneratorFileStream(int64_t key, int32_t version, const QString& baseName, const QString& format, qint32 maxFilesCount);

    template<class T>
    PropertiesFileNamesGeneratorFileStream& operator<<(T& data);
    PropertiesFileNamesGeneratorFileStream& operator<<(const PlainData& data);

private:
    void updateStream();

private:
    ScopedPointer<QFile> m_file;
    PropertiesFileNamesGenerator m_fileNamesGenerator;
    ScopedPointer<QStreamBufferWrite> m_stream;
    int64_t m_key;
    int32_t m_version;
};

template<class T>
inline PropertiesFileNamesGeneratorFileStream& PropertiesFileNamesGeneratorFileStream::operator<<(T& data)
{
    updateStream();
    if(m_stream != nullptr) {
        *m_stream << data;
    }
    return *this;
}

inline PropertiesFileNamesGeneratorFileStream& PropertiesFileNamesGeneratorFileStream::operator<<(const PlainData& data)
{
    updateStream();
    if(m_stream != nullptr) {
        *m_stream << data;
    }
    return *this;
}

#endif // PROPERTIESFILENAMESGENERATOR_H
