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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <MatrixCpp/Client.hpp>
#include <MatrixCpp/Responses.hpp>
#include <qjsondocument.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qurl.h>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

// Public definitions

Client::Client(const QUrl &homeserverUrl, QObject *parent)
    : Client(homeserverUrl.host()) {
    this->m_nam = new QNetworkAccessManager(this);
}

Client::Client(const QString &host, QObject *parent) : QObject(parent) {
    this->homeserverUrl = QUrl();
    this->homeserverUrl.setHost(host);
    this->homeserverUrl.setScheme("https"); // Always use https?
    this->m_nam = new QNetworkAccessManager(this);
}

// Api routines

ResponseFuture Client::getServerVersion() const {
    return this->get("/_matrix/client/versions");
}

ResponseFuture Client::getLoginTypes() const {
    return this->get("/_matrix/client/r0/login");
}

ResponseFuture Client::login(QString user, QString password, QString deviceId) {
    QVariantMap loginData, identifier;

    loginData["type"] = "m.login.password";

    identifier["type"] = "m.id.user";
    identifier["user"] = user;

    loginData["identifier"]                  = identifier;
    loginData["password"]                    = password;
    loginData["initial_device_display_name"] = APP_NAME;

    if (!deviceId.isEmpty())
        loginData["device_id"] = deviceId;

    ResponseFuture future = this->send("/_matrix/client/r0/login", loginData);

    QObject::connect(&future,
                     &ResponseFuture::responseComplete,
                     [&](LoginResponse response) {
                         if (response.isBroken() || response.isError())
                             return;

                         this->m_userId      = response.userId;
                         this->m_accessToken = response.accessToken;
                         this->m_deviceId    = response.deviceId;

                         if (!response.homeserver.isEmpty()) {
                             this->homeserverUrl = response.homeserver;
                             // TODO: response.identityServer
                         }
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

// Private

ResponseFuture Client::get(QString path) const {
    QUrl requestUrl = this->homeserverUrl;
    requestUrl.setPath(path);

    return this->get(requestUrl);
}

ResponseFuture Client::get(QUrl url) const {
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader,
                      APP_NAME " " APP_VERSION);

    qDebug() << "GET" << url;
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