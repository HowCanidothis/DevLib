#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QProgressBar>
#include <PropertiesModule/internal.hpp>

class ProgressBar : public QObject
{
    using Super = QObject;
public:
    ProgressBar(const Name& processId, QProgressBar* parent);

    DispatcherConnectionsSafe m_connections;
    qint32 m_counter;
};

#endif // PROGRESSBAR_H
