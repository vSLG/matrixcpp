/**
 * @file Response.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implementation of Response class
 * @version 0.1
 * @date 2021-01-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QJsonDocument>

#include <MatrixCpp/Responses.hpp>
#include <qjsondocument.h>

using namespace MatrixCpp::Responses;

// Public functions

Response::Response(QByteArray rawResponse) {
    // First parse JSON
    QJsonParseError error;
    QJsonDocument   doc = QJsonDocument::fromJson(rawResponse, &error);

    if (error.error) {
        qCritical() << "Failed to parse JSON response:"
                    << error.errorString().toStdString().c_str();
        this->m_broken = true;
        return;
    }

    this->data = doc.toVariant();
}

QByteArray Response::getJson() {
    QJsonDocument doc = QJsonDocument::fromVariant(this->data);
    return doc.toJson();
}