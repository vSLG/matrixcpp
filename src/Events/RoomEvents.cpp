// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file RoomEvents.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implementation of room-related events declared in Structs.hpp
 * @version 0.1
 * @date 2021-01-28
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <MatrixCpp/Types.hpp>

using namespace MatrixCpp::Types;

/*
 * EventContent
 */

void EventContent::parseData() {
    QVariantMap dataMap = this->data.toMap();
    BROKEN(dataMap["membership"].isNull())

    QString membership = dataMap["membership"].toString();
    this->membership   = MEMBERSHIP_UNKNOWN;

    if (membership == "invite")
        this->membership = MEMBERSHIP_INVITE;
    else if (membership == "join")
        this->membership = MEMBERSHIP_JOIN;
    else if (membership == "knock")
        this->membership = MEMBERSHIP_KNOCK;
    else if (membership == "leave")
        this->membership = MEMBERSHIP_LEAVE;
    else if (membership == "ban")
        this->membership = MEMBERSHIP_BAN;

    this->avatarUrl    = dataMap["avatar_url"].toString();
    this->displayName  = dataMap["displayname"].toString();
    this->isDirect     = dataMap["is_direct"].toBool();
    this->unsignedData = dataMap["unsigned"];
}

/*
 * NameContent
 */

void RoomNameContent::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->name = dataMap["name"].toString();
    BROKEN(this->name.isEmpty())
}

/*
 * RoomEvent
 */

void RoomEvent::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->eventId = dataMap["event_id"].toString();
    BROKEN(this->eventId.isEmpty())

    this->sender = dataMap["sender"].toString();
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