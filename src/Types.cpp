// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file Types.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implementation of many matrix-related data structures
 * @version 0.1
 * @date 2021-01-25
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QJsonDocument>

#include <MatrixCpp/Types.hpp>

using namespace MatrixCpp::Types;

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

    this->content = this->data.toMap()["content"].toMap();
    BROKEN(this->content.isEmpty())

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
    else if (type == "m.room.create")
        this->type = M_ROOM_CREATE;

    // Ephemeral events
    else if (type == "m.typing")
        this->type = M_TYPING;

    // Other types
    else
        this->type = M_OTHER;
}

/*
 * UnsignedData
 */

void UnsignedData::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->age             = dataMap["age"].toInt();
    this->redactedBecause = dataMap["redacted_because"];
    this->transactionId   = dataMap["transaction_id"].toString();
}

/*
 * CreateContent
 */
void CreateContent::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->creator = dataMap["creator"].toString();
    BROKEN(this->creator.isEmpty())

    this->federate    = dataMap["federate"].toBool();
    this->roomVersion = dataMap["room_version"].toString();

    this->roomId  = dataMap["predecessor"].toMap()["room_id"].toString();
    this->eventId = dataMap["predecessor"].toMap()["event_id"].toString();
}

/*
 * RoomUpdate
 */

void RoomUpdate::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->summary = dataMap["summary"].toMap();

    for (QVariant event : dataMap["state"].toMap()["events"].toList()) {
        StateEvent state = event;
        if (!state.isBroken())
            this->state.append(state);
    }

    this->timeline = dataMap["timeline"].toMap();

    for (QVariant event : dataMap["ephemeral"].toMap()["events"].toList()) {
        Event ephemeral = event;
        if (!ephemeral.isBroken())
            this->ephemeral.append(ephemeral);
    }

    for (QVariant event : dataMap["account_data"].toMap()["events"].toList()) {
        Event accountData = event;
        if (!accountData.isBroken())
            this->accountData.append(accountData);
    }

    this->unreadNotifications = dataMap["unread_notifications"].toMap();
}

/*
 * RoomInvite
 */

void RoomInvite::parseData() {
    QVariantMap inviteState = this->data.toMap()["invite_state"].toMap();

    for (QVariant event : inviteState["events"].toList()) {
        StrippedStateEvent stateEvent = event;
        if (!stateEvent.isBroken())
            this->events.append(stateEvent);
    }
}

/*
 * Rooms
 */

void Rooms::parseData() {
    QVariantMap dataMap = this->data.toMap();

    QVariantMap                 join = dataMap["join"].toMap();
    QVariantMap::const_iterator it   = join.begin();
    for (; it != join.end(); ++it) {
        RoomUpdate room = it.value();
        if (!room.isBroken())
            this->join.insert(it.key(), room);
    }

    QVariantMap invite = dataMap["invite"].toMap();
    it                 = invite.begin();
    for (; it != invite.end(); ++it) {
        RoomInvite room = it.value();
        if (!room.isBroken())
            this->invite.insert(it.key(), room);
    }

    this->leave = dataMap["leave"].toMap();
}

/*
 * User
 */

User::User(Room *         room,
           const QString &userId,
           const QString &displayName,
           const QString &avatarUrl)
    : QObject(room), userId(userId), avatarUrl(avatarUrl),
      displayName(displayName) {
}