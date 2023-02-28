#include "cloudservice.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>

namespace Routes {
static QString Login = "/auth/login";
}

CloudService::CloudService()
    : m_connection(new QNetworkAccessManager())
{
    m_connection->moveToThread(&m_thread);
}

CloudService::~CloudService()
{

}

void CloudService::Initialize()
{
    onInitialize();
    m_thread.start();
}

void CloudService::Terminate()
{
    onTerminate();
    m_thread.terminate();
    m_thread.wait();
}

AsyncRequest CloudService::Request(const CloudServiceRequestParams& params) const
{
    Q_ASSERT(m_thread.isRunning());
    QNetworkRequest request(QUrl(Params->Url + params.Route));
    request.setHeader(QNetworkRequest::ContentTypeHeader, Params->DefaultContentType);
    if(params.OnRequest != nullptr) {
        params.OnRequest(request);
    }
    AsyncRequest result;
    ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, m_connection.get(), [this, request, params, result]{
        if(result.ResultInterruptor.IsInterrupted()) {
            return;
        }
        QNetworkReply* reply = m_connection->sendCustomRequest(request, params.Verb, params.Payload);
        processReply(reply, params, result);
    });
    return result;
}


void CloudService::processReply(QNetworkReply* reply, const CloudServiceRequestParams& params, const AsyncRequest& result) const
{
    reply->connect(reply, &QNetworkReply::finished, [reply, result, params]{
        auto ok = false;
        guards::LambdaGuard guard([&]{ result.Result.Resolve(ok); reply->deleteLater(); });
        if(reply->error() != QNetworkReply::NoError) {
            if(params.OnError != nullptr) {
                params.OnError(reply->errorString());
            }            
            return;
        }
        params.OnReplied(result, *reply);
        ok = true;
    });
}

/*void CloudService::Authorize(const QString& user, const QString& password)
{
    if(!m_thread.isStarted()) {
        m_thread.start();
    }
    auto url = Url.Native();
    m_thread.RunTask().Then([url](bool ok){
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QJsonObject payload;
        payload.insert("email", user);
        payload.insert("password", password);
         QJsonDocument(payload).toJson();

    });
}*/

CloudServiceParams::CloudServiceParams()
    : Url("https://alpha-api.aidriller.com/api")
    , DefaultContentType("application/json")
{

}

CloudServiceRequestParams::CloudServiceRequestParams(const FOnReplied& onReplied)
    : OnReplied(onReplied)
    , Verb("GET")
{}

CloudServiceRequestParams::CloudServiceRequestParams(const FOnReplied& onReplied, const QByteArray& data)
    : OnReplied(onReplied)
    , Payload(data)
    , Verb("GET")
{

}

CloudServiceRequestParams& CloudServiceRequestParams::SetVerb(const QByteArray& verb)
{
    Verb = verb;
    return *this;
}

CloudServiceRequestParams& CloudServiceRequestParams::SetRoute(const QString& route)
{
    Route = route;
    return *this;
}

CloudServiceRequestParams& CloudServiceRequestParams::SetOnError(const FOnError& onError)
{
    OnError = onError;
    return *this;
}

CloudServiceRequestParams& CloudServiceRequestParams::SetOnRequest(const FOnRequest& onRequest)
{
    OnRequest = onRequest;
    return *this;
}
