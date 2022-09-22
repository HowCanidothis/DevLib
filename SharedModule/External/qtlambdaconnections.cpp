#include "qtlambdaconnections.h"

QtLambdaConnections::QtLambdaConnections()
{

}

QtLambdaConnections::~QtLambdaConnections()
{
    Clear();
}

void QtLambdaConnections::Add(QMetaObject::Connection connection)
{
    m_connections.append(connection);
}

void QtLambdaConnections::Clear()
{
     for(const auto& connection : m_connections) {
        QObject::disconnect(connection);
    }
    m_connections.clear();
}
