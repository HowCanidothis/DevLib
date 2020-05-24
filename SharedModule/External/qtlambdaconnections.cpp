#include "qtlambdaconnections.h"

QtLambdaConnections::QtLambdaConnections()
{

}

QtLambdaConnections::~QtLambdaConnections()
{
    for(const auto& connection : m_connections) {
        QObject::disconnect(connection);
    }
}
