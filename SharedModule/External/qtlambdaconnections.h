#ifndef QTLAMBDACONNECTIONS_H
#define QTLAMBDACONNECTIONS_H

#include <QObject>
#include <QMetaObject>
#include <QVector>

class QtLambdaConnections
{
public:
    QtLambdaConnections();
    ~QtLambdaConnections();

    template<typename ... Args>
    void connect(Args... args);
    void Add(QMetaObject::Connection);
    void Clear();
    bool IsEmpty() const { return m_connections.isEmpty(); }

private:
    QVector<QMetaObject::Connection> m_connections;
};

template<typename ... Args>
void QtLambdaConnections::connect(Args... args)
{
    m_connections.append(QObject::connect(args...));
}

#endif // QTLAMBDACONNECTIONS_H
