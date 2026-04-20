#include "cloudservice.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHttpMultiPart>

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

AsyncRequest CloudService::PostFile(const CloudServiceRequestParams& params, const QString& filePath) const
{
    AsyncRequest result;
    if(!m_thread.isRunning()) {
        result.Result.Resolve(false);
        return result;
    }
    QNetworkRequest request(QUrl(Params->Url + params.Route));
    if(params.OnRequest != nullptr) {
        params.OnRequest(request);
    }

    ThreadsBase::DoQThreadWorkerWithResult(CONNECTION_DEBUG_LOCATION, m_connection.get(), [this, request, params, result, filePath]{
        QFile* file = new QFile(filePath);
        if (!file->open(QIODevice::ReadOnly)) {
            result.Result.Resolve(false);
            return;
        }

        QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart filePart;
        // Set headers for the file part
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
            QVariant("form-data; name=\"file\"; filename=\"" + file->fileName() + "\""));
        filePart.setBodyDevice(file);
        file->setParent(multiPart); // File is deleted when multiPart is deleted
        multiPart->append(filePart);

        if(params.OnMultiPart != nullptr) {
            params.OnMultiPart(*multiPart);
        }

        if(result.ResultInterruptor.IsInterrupted()) {
            multiPart->deleteLater();
            return;
        }

        QNetworkReply* reply = m_connection->post(request, multiPart);
        multiPart->setParent(reply);
        processReply(reply, params, result);
    });
    return result;
}

AsyncRequest CloudService::Request(const CloudServiceRequestParams& params) const
{
    AsyncRequest result;
    if(!m_thread.isRunning()) {
        result.Result.Resolve(false);
        return result;
    }
    QNetworkRequest request(QUrl(Params->Url + params.Route));
    request.setHeader(QNetworkRequest::ContentTypeHeader, Params->DefaultContentType);
    for(const auto& headerPair : params.RawHeaders) {
        request.setRawHeader(headerPair.first, headerPair.second);
    }
    if(params.OnRequest != nullptr) {
        params.OnRequest(request);
    }

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
                params.OnError(reply->errorString() + ": " + reply->readAll());
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
    : DefaultContentType("application/json")
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
