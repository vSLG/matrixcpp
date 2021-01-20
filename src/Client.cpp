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
#include <qnetworkreply.h>
#include <qnetworkrequest.h>
#include <qobject.h>

using namespace MatrixCpp;

// Private functions and variables

struct Client::Privates {
    explicit Privates(Client *parent = nullptr) : client(parent) {
    }

    Client *               client;
    QNetworkAccessManager *m_nam;

    QNetworkReply *get(QString url) {
        return this->get(QUrl(url));
    }

    QNetworkReply *get(QUrl url) {
        QNetworkRequest request(url);

        request.setHeader(QNetworkRequest::UserAgentHeader, "MatrixCpp 0.1");

        QNetworkReply *reply = this->m_nam->get(request);

        QObject::connect(
            this->client, SIGNAL(abortRequests()), reply, SLOT(abort()));

        return reply;
    }
};

// Public definitions

Client::Client(const QUrl &homeserverUrl, QObject *parent)
    : QObject(parent), m_private(new Privates), homeserverUrl(homeserverUrl) {
    m_private->m_nam = new QNetworkAccessManager(this);
}

Client::~Client() {
    delete m_private;
}