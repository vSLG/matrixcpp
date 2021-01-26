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
 * EventContent
 */

void EventContent::parseData() {
    QVariantMap dataMap = this->data.toMap();
    BROKEN(dataMap["membership"].isNull())

    this->membership   = dataMap["membership"].toString();
    this->avatarUrl    = dataMap["avatar_url"].toString();
    this->displayName  = dataMap["displayname"].toString();
    this->isDirect     = dataMap["is_direct"].toBool();
    this->unsignedData = dataMap["unsigned"];
}

/*
 * RoomEvent
 */

void RoomEvent::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->content = dataMap["content"].toMap();
    BROKEN(this->content.isEmpty())

    this->eventId = dataMap["event_id"].toString();
    BROKEN(this->eventId.isEmpty())

    this->sender = dataMap["seneder"].toString();
    BROKEN(this->sender.isEmpty())

    this->serverTs = dataMap["origin_server_ts"].toInt();
    BROKEN(this->serverTs == 0)

    this->unsignedData = dataMap["unsigned"];
}

/*
 * StateEvent
 */

void StateEvent::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->stateKey    = dataMap["state_key"].toString();
    this->prevContent = dataMap["prev_content"];
}

/*
 * StrippeedStateEvent
 */

void StrippedStateEvent::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->content = dataMap["content"];
    BROKEN(this->content.isBroken())

    this->sender = dataMap["seneder"].toString();
    BROKEN(this->sender.isEmpty())

    this->stateKey = dataMap["state_key"].toString();
}

/*
 * Room
 */

void Room::parseData() {
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
        Room room = it.value();
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