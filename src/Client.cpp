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
#include <qnetworkrequest.h>
#include <qurl.h>

using namespace MatrixCpp;
using namespace MatrixCpp::Responses;

// Public definitions

Client::Client(const QUrl &homeserverUrl, QObject *parent)
    : QObject(parent), homeserverUrl(homeserverUrl) {
    this->m_nam = new QNetworkAccessManager(this);
}

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