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

using namespace MatrixCpp::Responses;

// Public functions

ResponseFuture::ResponseFuture(QNetworkReply *reply) : QObject(nullptr) {
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        this->m_finished = true;

        if (reply->error()) {
            emit this->errorOccurred(reply->errorString());
            this->m_rawResponse = "";
        } else
            this->m_rawResponse = reply->readAll();

        emit this->responseComplete(this->result());
    });
}

Response ResponseFuture::result() {
    // Wait for response if it's not completed
    if (!this->m_finished) {
        QEventLoop loop;
        QObject::connect(
            this, SIGNAL(responseComplete(Response)), &loop, SLOT(quit()));
        loop.exec();
    }

    return Response(this->m_rawResponse);
}