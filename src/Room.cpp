// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file Room.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implements Room, which is used by Client
 * @version 0.1
 * @date 2021-01-28
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <QDebug>

#include <MatrixCpp/Room.hpp>

using namespace MatrixCpp::Types;

Room::Room(const QString &roomId, Client *client)
    : QObject((QObject *) client), roomId(roomId) {
}

QString Room::name() const {
    if (!this->m_name.isEmpty())
        return this->m_name;
    return this->roomId;
}

void Room::onEvent(RoomEvent event) {
    qDebug() << "ROOM" << this->name()
             << "EVENT:" << event.data.toMap()["type"].toString();

    switch (event.type) {
        case Event::M_ROOM_MEMBER:
            this->onRoomMemberEvent(event);
            break;
        case Event::M_ROOM_CREATE: {
            CreateContent content(event.content);

            if (!this->users.contains(content.creator))
                this->updateMember(content.creator);

            this->creator  = this->users.value(content.creator);
            this->federate = content.federate;
            break;
        }
        case Event::M_ROOM_NAME:
            this->m_name = ((NameContent) event.content).name;
            break;
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

        qDebug() << "ROOM" << this->name() << "ADD:" << user->userId;

        // We have nothing left to do
        return;
    }

    // Now update the user accordingly
    User *user = nullptr;

    if (membership == EventContent::MEMBERSHIP_JOIN &&
        this->invitedUsers.contains(userId)) {
        // User changed from invited -> join
        user = this->invitedUsers.take(userId);
        this->users.insert(userId, user);
    } else
        user = this->users[userId];

    if (!user) {
        qCritical() << "User broken" << userId;
        return;
    }

    if (!displayName.isEmpty())
        user->displayName = displayName;

    if (!avatarUrl.isEmpty())
        user->avatarUrl = avatarUrl;
}