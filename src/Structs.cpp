/**
 * @file Structs.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implementation of many matrix-related data structures
 * @version 0.1
 * @date 2021-01-25
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QJsonDocument>

#include <MatrixCpp/Structs.hpp>

using namespace MatrixCpp::Structs;

/*
 * MatrixObj
 */

MatrixObj::MatrixObj() : m_broken(true){};

MatrixObj::MatrixObj(QByteArray rawJson) {
    BROKEN(rawJson.isEmpty())

    // First parse JSON
    QJsonParseError error;
    QJsonDocument   doc = QJsonDocument::fromJson(rawJson, &error);

    BROKEN(error.error)

    this->data = doc.toVariant();
}

MatrixObj::MatrixObj(QVariant data) {
    this->data = data;
}

MatrixObj::MatrixObj(const MatrixObj &other) {
    this->data     = other.data;
    this->m_broken = other.m_broken;
}

QByteArray MatrixObj::getJson() const {
    QJsonDocument doc = QJsonDocument::fromVariant(this->data);
    return doc.toJson();
}

bool MatrixObj::isBroken() const {
    return this->m_broken;
}

/*
 * Event
 */

void Event::parseData() {
    QString type = this->data.toMap()["type"].toString();
    BROKEN(type.isEmpty())

    // Presence events
    if (type == "m.presence")
        this->type = M_PRESENCE;

    // Room events
    else if (type == "m.room.member")
        this->type = M_ROOM_MEMBER;
    else if (type == "m.room.message")
        this->type = M_ROOM_MESSAGE;
    else if (type == "m.room.name")
        this->type = M_ROOM_NAME;

    // Ephemeral events
    else if (type == "m.typing")
        this->type = M_TYPING;

    // Other types
    else
        this->type = M_OTHER;
}