/**
 * @file BaseResponse.cpp
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

using namespace MatrixCpp::Responses;

// Public functions

Response::Response(QByteArray rawResponse) {
    if (rawResponse.isEmpty()) {
        qCritical() << "Response: Response string is empty";
        this->m_broken = this->m_deepBroken = true;
        return;
    }

    // First parse JSON
    QJsonParseError error;
    QJsonDocument   doc = QJsonDocument::fromJson(rawResponse, &error);

    if (error.error) {
        qCritical() << "Response: Failed to parse JSON response:"
                    << error.errorString().toStdString().c_str();
        this->m_broken = this->m_deepBroken = true;
        return;
    }

    this->data = doc.toVariant();
}

Response::Response(QVariant data) {
    this->data = data;
}

Response::Response(const Response &other) {
    this->data         = other.data;
    this->m_deepBroken = other.m_deepBroken;
}

void Response::parseData() {
    // Simply return. Subclasses must override this function
    return;
}

QByteArray Response::getJson() const {
    QJsonDocument doc = QJsonDocument::fromVariant(this->data);
    return doc.toJson();
}

bool Response::isBroken() const {
    return this->m_broken || this->m_deepBroken;
}

bool Response::isError() const {
    return !this->data.toMap()["errcode"].isNull();
}