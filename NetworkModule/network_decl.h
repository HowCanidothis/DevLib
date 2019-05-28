#ifndef NETWORK_DECL_H
#define NETWORK_DECL_H

#include <Qt>

class NetworkPackage;

class INetworkConnectionOutput
{
public:
    virtual ~INetworkConnectionOutput(){}

protected:
    friend class NetworkConnection;
    // Note: the function is executing in the thread pool
    virtual void onPackageRecieved(qintptr descriptor, const NetworkPackage& package) = 0;
    virtual void onConnectionLost(qintptr descriptor) = 0;
};

#endif // NETWORK_DECL_H
