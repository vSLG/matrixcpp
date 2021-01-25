/**
 * @file Client.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implements Client
 * @version 0.1
 * @date 2021-01-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QException>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <MatrixCpp/Client.hpp>
#include <MatrixCpp/Responses.hpp>
#include <qobject.h>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

// Public definitions

Client::Client(const QUrl &   homeserverUrl,
               const QString &user,
               const QString &deviceId,
               QObject *      parent)
    : Client(homeserverUrl.host(), user, deviceId, parent) {
}

Client::Client(const QString &host,
               const QString &user,
               const QString &deviceId,
               QObject *      parent)
    : QObject(parent), m_userId(user), m_deviceId(deviceId) {
    this->homeserverUrl.setHost(host);
    this->homeserverUrl.setScheme("https"); // Always use https?
    this->m_nam = new QNetworkAccessManager(this);
}

void Client::loadDiscovery() {
    // Simply invoke getDiscovery and wait for it
    this->getDiscovery().result();
}

void Client::restore(const QString &userId,
                     const QString &deviceId,
                     const QString &accessToken) {
    this->m_userId      = userId;
    this->m_deviceId    = deviceId;
    this->m_accessToken = accessToken;
}

// Api routines

ResponseFuture Client::getDiscovery() {
    ResponseFuture future = this->get("/.well-known/matrix/client");

    QObject::connect(
        &future, &ResponseFuture::responseComplete, [=](Response response) {
            this->onDiscoveryResponse(response);
        });

    return future;
}

ResponseFuture Client::getServerVersion() const {
    return this->get("/_matrix/client/versions");
}

ResponseFuture Client::getLoginTypes() const {
    return this->get("/_matrix/client/r0/login");
}

ResponseFuture Client::login(QString password, QString token) {
    QVariantMap loginData;

    if (this->m_userId.isEmpty())
        throw std::runtime_error("Please set an user id");

    if (!password.isEmpty()) {
        QVariantMap identifier;

        identifier["type"] = "m.id.user";
        identifier["user"] = this->m_userId;

        loginData["type"]       = "m.login.password";
        loginData["password"]   = password;
        loginData["identifier"] = identifier;
    } else if (!token.isEmpty()) {
        loginData["type"]  = "m.login.token";
        loginData["token"] = token;
    } else
        throw std::runtime_error("Please provide a password or token");

    loginData["initial_device_display_name"] = APP_NAME;

    ResponseFuture future = this->send("/_matrix/client/r0/login", loginData);

    QObject::connect(
        &future, &ResponseFuture::responseComplete, [=](Response response) {
            this->onLoginResponse(response);
        });

    return future;
}

ResponseFuture Client::sync(const QString &filter,
                            const QString &since,
                            bool           fullState,
                            Presence       presence,
                            int            timeout) {
    QUrlQuery query;

    query.addQueryItem("access_token", this->m_accessToken);

    if (!filter.isEmpty())
        query.addQueryItem("filter", filter);

    if (!since.isEmpty())
        query.addQueryItem("since", since);
    else if (!this->m_nextBatch.isEmpty())
        query.addQueryItem("since", this->m_nextBatch);

    query.addQueryItem("full_state", fullState ? "true" : "false");
    query.addQueryItem("presence",
                       presence == PRESENCE_ONLINE        ? "online"
                       : presence == PRESENCE_UNAVAILABLE ? "unavailable"
                                                          : "offline");

    query.addQueryItem("timeout", QString::number(timeout));

    ResponseFuture future = this->get("/_matrix/client/r0/sync", query);

    QObject::connect(
        &future, &ResponseFuture::responseComplete, [=](Response response) {
            this->onSyncResponse(response);
        });

    return future;
}

// Getters & setters

QString Client::userId() const {
    return this->m_userId;
}

QString Client::deviceId() const {
    return this->m_deviceId;
}

QString Client::accessToken() const {
    return this->m_accessToken;
}

// Protected slots

void Client::onLoginResponse(LoginResponse response) {
    if (response.isBroken() || response.isError())
        return;

    if (this->homeserverUrl != response.homeserver)
        this->homeserverUrl = response.homeserver;

    this->restore(response.userId, response.deviceId, response.accessToken);
}

void Client::onDiscoveryResponse(WellKnownResponse response) {
    if (response.isBroken() || response.isError())
        return;

    // Do we really need to check returned homeserver URL?
    this->homeserverUrl = response.homeserver;
    // TODO: response.identityServer
}

void Client::onSyncResponse(SyncResponse response) {
    this->m_nextBatch = response.nextBatch;
}

// Private

ResponseFuture Client::get(QString path, QUrlQuery query) const {
    QUrl requestUrl = this->homeserverUrl;
    requestUrl.setPath(path);
    requestUrl.setQuery(query);

    return this->get(requestUrl);
}

ResponseFuture Client::get(QUrl url) const {
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader,
                      APP_NAME " " APP_VERSION);

    qDebug() << "GET" << url.path();
    QNetworkReply *reply = this->m_nam->get(request);

    QObject::connect(this, SIGNAL(abortRequests()), reply, SLOT(abort()));

    return ResponseFuture(reply);
}

ResponseFuture Client::send(QString path, QVariant data) const {
    QUrl requestUrl = this->homeserverUrl;
    requestUrl.setPath(path);

    return this->send(requestUrl, data);
}

ResponseFuture Client::send(QUrl url, QVariant data) const {
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      APP_NAME " " APP_VERSION);

    QByteArray postData = QJsonDocument::fromVariant(data).toJson();

    qDebug() << "POST" << url;
    QNetworkReply *reply = this->m_nam->post(request, postData);

    QObject::connect(this, SIGNAL(abortRequests()), reply, SLOT(abort()));

    return ResponseFuture(reply);
}