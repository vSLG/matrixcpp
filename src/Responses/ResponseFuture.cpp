// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file ResponseFuture.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implementation of ResponseFuture class
 * @version 0.1
 * @date 2021-01-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QEventLoop>
#include <QNetworkReply>

#include <MatrixCpp/Responses.hpp>
#include <qnamespace.h>
#include <qnetworkaccessmanager.h>

using namespace MatrixCpp::Responses;

// Public functions

ResponseFuture::ResponseFuture(QNetworkReply *reply) : QObject(nullptr) {
    this->init(reply);
}

ResponseFuture::ResponseFuture(const ResponseFuture &other) {
    this->init(other.m_reply);
}

Response ResponseFuture::result() {
    // Wait for response if it's not completed
    if (!this->m_finished) {
        QEventLoop loop;
        QObject::connect(
            this, SIGNAL(responseComplete(Response)), &loop, SLOT(quit()));
        loop.exec();
    }

    this->deleteLater();
    return Response(this->m_rawResponse);
}

// Private functions

void ResponseFuture::init(QNetworkReply *reply) {
    this->m_reply = reply;

    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        this->m_finished    = true;
        this->m_rawResponse = reply->readAll();
        reply->deleteLater();
        emit this->responseComplete(this->result());
    });
}