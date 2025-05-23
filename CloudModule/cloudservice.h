#ifndef CLOUDSERVICE_H
#define CLOUDSERVICE_H

#include <PropertiesModule/internal.hpp>

class CloudServiceParams
{
public:
    CloudServiceParams();

    QString Url;
    QString DefaultContentType;
};

using CloudServiceParamsPtr = SharedPointer<CloudServiceParams>;

class QNetworkAccessManager;

struct CloudServiceRequestParams
{
    using FOnRequest = std::function<void (class QNetworkRequest&)>;
    using FOnReplied = std::function<void (const AsyncRequest&, class QNetworkReply&)>;
    using FOnError = std::function<void (const QString&)>;

    FOnReplied OnReplied;
    FOnError OnError;
    FOnRequest OnRequest;
    QByteArray Payload;
    QString Route;
    QByteArray Verb;
    QVector<std::pair<QByteArray, QByteArray>> RawHeaders;

    CloudServiceRequestParams(const FOnReplied& onReplied);
    CloudServiceRequestParams(const FOnReplied& onReplied, const QByteArray& data);

    CloudServiceRequestParams& SetRawHeaders(const QVector<std::pair<QByteArray, QByteArray>>& rawHeaders) { RawHeaders = rawHeaders; return *this; }
    CloudServiceRequestParams& SetPostVerb() { Verb = "POST"; return *this; }
    CloudServiceRequestParams& SetVerb(const QByteArray& verb);
    CloudServiceRequestParams& SetRoute(const QString& route);
    CloudServiceRequestParams& SetOnError(const FOnError& onError);
    CloudServiceRequestParams& SetOnRequest(const FOnRequest& onRequest);
};

class CloudService
{
public:
    CloudService();
    virtual ~CloudService();

    SharedPointerInitialized<CloudServiceParams> Params;

    void Initialize();
    void Terminate();

    AsyncRequest Request(const CloudServiceRequestParams& params) const;

    //CommonDispatcher<qint64, const class QNetworkReply&> OnReplyGot;

protected:
    virtual void onInitialize() {}
    virtual void onTerminate() {}

private:
    void processReply(QNetworkReply* reply, const CloudServiceRequestParams& params, const AsyncRequest& result) const;

private:
    mutable ScopedPointer<QNetworkAccessManager> m_connection;
    QThread m_thread;
};

#endif // CLOUDSERVICE_H
