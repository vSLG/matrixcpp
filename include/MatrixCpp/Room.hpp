// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file Room.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Declares Room class, used by Client
 * @version 0.1
 * @date 2021-01-28
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QObject>

#include <MatrixCpp/Types.hpp>
#include <MatrixCpp/export.hpp>

namespace MatrixCpp::Types {

/**
 * @brief A Matrix room, used by Client to store Room information
 *
 */
class PUBLIC Room : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new Room
     *
     * @param roomId
     * @param client The Client this Room is registered
     */
    Room(const QString &roomId, Client *client = nullptr);

    /**
     * @brief Returns Room's name
     *
     * @return QString
     */
    QString name() const;

    QString               roomId;       ///< This Room's ID
    QMap<QString, User *> users;        ///< Users this Room has
    QMap<QString, User *> invitedUsers; ///< Users invited to this Room
    User *                creator;      ///< The creator of this Room
    bool federate = true; ///< Whether users on other servers can join this Room

  protected:
    /**
     * @brief Update or add a member to this room
     *
     * @param userId
     * @param displayName
     * @param avatarUrl
     * @param membership One of: MEMBERSHIP_JOIN, MEMBERSHIP_INVITE
     */
    void updateMember(
        const QString &          userId,
        const QString &          displayName = "",
        const QString &          avatarUrl   = "",
        EventContent::Membership membership  = EventContent::MEMBERSHIP_JOIN);

  public slots:
    /**
     * @brief Process StateEvent and update Room accordingly
     *
     * @param event
     */
    void onEvent(RoomEvent event);

    /**
     * @brief Process m.room.member events
     *
     * @param event
     */
    void onRoomMemberEvent(StateEvent event);

  private:
    QString m_name;
};
} // namespace MatrixCpp::Types