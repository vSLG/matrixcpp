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

ResponseFuture Client::getServerVersion() {
    return this->get("/_matrix/client/versions");
}

// Private

ResponseFuture Client::get(QString path) {
    QUrl requestUrl = this->homeserverUrl;
    requestUrl.setPath(path);

    qDebug() << "Getting" << requestUrl;
    return this->get(requestUrl);
}

ResponseFuture Client::get(QUrl url) {
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::UserAgentHeader,
                      APP_NAME " " APP_VERSION);

    QNetworkReply *reply = this->m_nam->get(request);

    QObject::connect(this, SIGNAL(abortRequests()), reply, SLOT(abort()));

    return ResponseFuture(reply);
}