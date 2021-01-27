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
#include <qdebug.h>
#include <qobject.h>

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
 * RoomEvent
 */

void RoomEvent::parseData() {
    QVariantMap dataMap = this->data.toMap();

    this->content = dataMap["content"].toMap();
    BROKEN(this->content.isEmpty())

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
 * Room
 */

Room::Room(const QString &roomId, Client *client)
    : QObject((QObject *) client), m_roomId(roomId) {
}

void Room::onStateEvent(StateEvent event) {
    switch (event.type) {
        case Event::M_ROOM_MEMBER:
            this->onRoomMemberEvent(event);
            break;
        case Event::M_ROOM_CREATE: {
            CreateContent content(event.content);

            if (!this->users.contains(content.creator))
                this->updateMember(content.creator);

            this->creator = this->users[content.creator];
            break;
        }
        default:
            qDebug() << __FUNCTION__ << "Implement me:"
                     << event.data.toMap()["type"].toString();
    }
}

void Room::onRoomMemberEvent(StateEvent event) {
    EventContent content(event.content);

    switch (content.membership) {
        case EventContent::MEMBERSHIP_JOIN:
        case EventContent::MEMBERSHIP_INVITE:
            this->updateMember(event.stateKey,
                               content.displayName,
                               content.avatarUrl,
                               content.membership);
            break;
        case EventContent::MEMBERSHIP_BAN:
        case EventContent::MEMBERSHIP_LEAVE: {
            User *user;
            user = this->users.take(event.stateKey);
            delete user;
            user = this->invitedUsers.take(event.stateKey);
            delete user;
            break;
        }
        default:
            break;
    }
}

void Room::updateMember(const QString &          userId,
                        const QString &          displayName,
                        const QString &          avatarUrl,
                        EventContent::Membership membership) {
    // Start by checking if we know this member
    // If we do not know, add them
    if (!this->users.contains(userId) && !this->invitedUsers.contains(userId)) {
        User *user = new User(this, userId, displayName, avatarUrl);
        if (membership == EventContent::MEMBERSHIP_JOIN)
            this->users.insert(userId, user);
        else
            this->invitedUsers.insert(userId, user);

        qDebug() << this->m_roomId << "ADD:" << user->userId;

        // We have nothing left to do
        return;
    }

    // Now update the user accordingly
    User *user;

    if (membership == EventContent::MEMBERSHIP_JOIN &&
        this->invitedUsers.contains(userId)) {
        // User changed from invited -> join
        user = this->invitedUsers.take(userId);
        this->users.insert(userId, user);
    } else
        user = this->users[userId];

    if (!displayName.isEmpty())
        user->displayName = displayName;

    if (!avatarUrl.isEmpty())
        user->avatarUrl = avatarUrl;
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