// SPDX-License-Identifier: LGPL-3.0-or-later

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

#include "Olm.hpp"

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;
using namespace MatrixCpp::Types;
using namespace MatrixCpp::Crypto;

// Public definitions

Client::Client(const QUrl &homeserverUrl, bool encryption, QObject *parent)
    : QObject(parent), homeserverUrl(homeserverUrl), m_encryption(encryption),
      m_nam(new QNetworkAccessManager(this)) {
}

/* Client::Client(const QString &host,
               const QString &user,
               const QString &deviceId,
               QObject *      parent)
    : Client(homeserverUrl.host(), user, deviceId, parent) {
    this->homeserverUrl.setHost(host);
    this->homeserverUrl.setScheme("http"); // Always use https?
    this->m_nam = new QNetworkAccessManager(this);
} */

void Client::loadDiscovery() {
    // Simply invoke getDiscovery and wait for it
    this->getDiscovery()->result();
}

void Client::restore(const QString &userId,
                     const QString &deviceId,
                     const QString &accessToken) {
    this->m_userId      = userId;
    this->deviceId      = deviceId;
    this->m_accessToken = accessToken;

    if (this->m_encryption)
        this->m_olm = new Olm(this);
}

// Api routines

ResponseFuture *Client::getDiscovery() {
    ResponseFuture *future = this->get("/.well-known/matrix/client");

    QObject::connect(
        future, &ResponseFuture::responseComplete, [=](Response response) {
            this->onDiscoveryResponse(response);
        });

    return future;
}

ResponseFuture *Client::getServerVersion() const {
    return this->get("/_matrix/client/versions");
}

ResponseFuture *Client::getLoginTypes() const {
    return this->get("/_matrix/client/r0/login");
}

ResponseFuture *Client::login(QString userId, QString password, QString token) {
    QVariantMap loginData;

    if (this->m_userId.isEmpty() && userId.isEmpty())
        throw std::runtime_error("Please set an user id");

    if (!password.isEmpty()) {
        QVariantMap identifier;

        identifier["type"] = "m.id.user";
        identifier["user"] = this->m_userId.isEmpty() ? userId : this->m_userId;

        loginData["type"]       = "m.login.password";
        loginData["password"]   = password;
        loginData["identifier"] = identifier;
    } else if (!token.isEmpty()) {
        loginData["type"]  = "m.login.token";
        loginData["token"] = token;
    } else
        throw std::runtime_error("Please provide a password or token");

    loginData["initial_device_display_name"] = APP_NAME;

    ResponseFuture *future = this->send("/_matrix/client/r0/login", loginData);

    QObject::connect(
        future, &ResponseFuture::responseComplete, [=](Response response) {
            this->onLoginResponse(response);
        });

    return future;
}

ResponseFuture *Client::sync(const QString &filter,
                             const QString &since,
                             bool           fullState,
                             Presence       presence,
                             int            timeout) {
    // Send keys if not sent yet and wait for it
    if (this->m_olm && !this->m_olm->deviceKeysUploaded)
        this->m_olm->sendKeys()->result();

    QUrlQuery query;

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

    ResponseFuture *future = this->get("/_matrix/client/r0/sync", query);

    QObject::connect(
        future, &ResponseFuture::responseComplete, [=](Response response) {
            this->onSyncResponse(response);
        });

    return future;
}

ResponseFuture *Client::send(QString path, QVariantMap data) const {
    QUrl requestUrl = this->homeserverUrl;
    requestUrl.setPath(path);

    return this->send(requestUrl, data);
}

ResponseFuture *Client::get(QString path, QUrlQuery query) const {
    QUrl requestUrl = this->homeserverUrl;
    requestUrl.setPath(path);
    requestUrl.setQuery(query);

    return this->get(requestUrl);
}

// Getters & setters

QString Client::userId() const {
    return this->m_userId;
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
    if (response.isBroken() || response.isError())
        return;

    this->m_nextBatch = response.nextBatch;

    if (!response.rooms.join.isEmpty())
        this->onRoomJoinUpdate(response.rooms.join);

    // From now on handle olm stuff
    if (!this->m_encryption)
        return;

    this->m_olm->uploadedOneTimeKeys =
        response.deviceOneTimeKeysCount["signed_curve25519"].toInt();

    // Upload one time keys if needed
    if (this->m_olm->shouldUploadOneTimeKeys())
        this->m_olm->sendKeys()
            ->result(); // We should wait until next sync call
}

void Client::onRoomJoinUpdate(const QMap<QString, RoomUpdate> &roomsUpdates) {
    QMap<QString, RoomUpdate>::const_iterator it = roomsUpdates.begin();
    do {
        // TODO: check if room is on invites

        if (!this->rooms.contains(it.key()))
            this->rooms.insert(it.key(), new Room(it.key(), this));

        Room *room = this->rooms[it.key()];

        for (StateEvent event : it.value().state)
            room->onEvent(event);

        for (RoomEvent event : it.value().timeline["events"].toList()) {
            room->onEvent(event);
        }
    } while (++it != roomsUpdates.end());
}

// Private

ResponseFuture *Client::get(QUrl url) const {
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader,
                      APP_NAME " " APP_VERSION);

    if (!this->m_accessToken.isEmpty())
        request.setRawHeader("Authorization",
                             "Bearer " + this->m_accessToken.toUtf8());

    qDebug() << "GET" << url.path();
    QNetworkReply *reply = this->m_nam->get(request);

    QObject::connect(this, SIGNAL(abortRequests()), reply, SLOT(abort()));

    return new ResponseFuture(reply);
}

ResponseFuture *Client::send(QUrl url, QVariantMap data) const {
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      APP_NAME " " APP_VERSION);

    if (!this->m_accessToken.isEmpty())
        request.setRawHeader("Authorization",
                             "Bearer " + this->m_accessToken.toUtf8());

    QByteArray postData = QJsonDocument::fromVariant(data).toJson();

    qDebug() << "POST" << url.path();
    QNetworkReply *reply = this->m_nam->post(request, postData);

    QObject::connect(this, SIGNAL(abortRequests()), reply, SLOT(abort()));

    return new ResponseFuture(reply);
}