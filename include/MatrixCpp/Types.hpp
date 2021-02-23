// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file Types.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Declares many Matrix-related data structures
 * @version 0.1
 * @date 2021-01-25
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QVariant>

#include <MatrixCpp/export.hpp>

/**
 * @brief Generates basic header for specified subclass
 *
 */
#define CLASS_CONSTRUCTOR(type, parent)                  \
  protected:                                             \
    virtual void parseData() override;                   \
                                                         \
  public:                                                \
    type(QByteArray rawResponse) : parent(rawResponse) { \
        if (!this->isBroken())                           \
            this->parseData();                           \
    };                                                   \
    type(QVariant data) : parent(data) {                 \
        if (!this->isBroken())                           \
            this->parseData();                           \
    };                                                   \
    type(const parent &other) : parent(other) {          \
        if (!this->isBroken())                           \
            this->parseData();                           \
    };

#define MATRIXOBJ_CONSTRUCTOR(type) CLASS_CONSTRUCTOR(type, MatrixObj)

/**
 * @brief Sets Response broken property if cond is true
 *
 * @param cond Condition to set Response broken property
 */
#define BROKEN(cond)           \
    if (cond) {                \
        this->m_broken = true; \
        return;                \
    }

namespace MatrixCpp {
// We cannot include Client.hpp here, so forward declaration
class Client;

namespace Types {

// Forward declaration
class PUBLIC MatrixObj;

// Room related non-events
class PUBLIC RoomUpdate;
class PUBLIC RoomInvite;
class PUBLIC Rooms;

// Event has to be defined before the below
class PUBLIC Event;

// Event related classes
class PUBLIC UnsignedData;
class PUBLIC EventContent;
class PUBLIC CreateContent;
class PUBLIC RoomNameContent;

// Events
class PUBLIC RoomEvent;
class PUBLIC StateEvent;
class PUBLIC StrippedStateEvent;

class PUBLIC Room;
class PUBLIC User;

/**
 * @brief Base class for every matrix object (JSON)
 *
 */
class PUBLIC MatrixObj {
  public:
    /**
     * @brief Construct a new empty MatrixObj
     *
     */
    explicit MatrixObj();

    /**
     * @brief Construct a new MatrixObj from JSON string
     *
     * @param rawJson The JSON string
     */
    explicit MatrixObj(QByteArray rawJson);

    /**
     * @brief Construct a new MatrixObj from QVariant data
     *
     * @param data
     */
    explicit MatrixObj(QVariant data);

    /**
     * @brief Construct a new MatrixObj object based on other MatrixObj
     *
     * @param other
     */
    explicit MatrixObj(const MatrixObj &other);

    /**
     * @brief Tell if data is broken (e.g. malformed or empty json)
     *
     * @return true
     * @return false
     */
    bool isBroken() const;

    /**
     * @brief Get the JSON-formatted data
     *
     * @return QByteArray
     */
    QByteArray getJson() const;

    QVariant data; ///< Original data stored in this MatrixObj

  protected:
    /**
     * @brief Function to be run after parent constructor, for parsing given
       data
     *
     */
    virtual void parseData() = 0;

    bool m_broken = false;
};

/**
 * @brief A matrix Room object
 *
 */
class PUBLIC RoomUpdate : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(RoomUpdate)

  public:
    /**
     * @brief Information about the room which clients may need to correctly
       render it to users
     *
     */
    QVariantMap summary;

    /**
     * @brief Updates to the state, between the time indicated by the since
       parameter, and the start of the timeline
     *
     */
    QList<StateEvent> state;

    /**
     * @brief The timeline of messages and state changes in the room
     *
     */
    QVariantMap timeline;

    /**
     * @brief The ephemeral events in the room that aren't recorded in the
       timeline or state of the room. e.g. typing
     *
     */
    QList<Event> ephemeral;

    /**
     * @brief The private data that this user has attached to this room
     *
     */
    QList<Event> accountData;

    /**
     * @brief Counts of unread notifications for this room
     *
     */
    QVariantMap unreadNotifications;
};

/**
 * @brief A Room which user has been invited to
 *
 */
class PUBLIC RoomInvite : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(RoomInvite)

  public:
    /**
     * @brief The StrippedState events that form the invite state
     *
     */
    QList<StrippedStateEvent> events;
};

/**
 * @brief Contains rooms information. Returned on sync request
 *
 */
class PUBLIC Rooms : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(Rooms)

  public:
    Rooms(){};

    QMap<QString, RoomUpdate> join; ///< The rooms that the user has joined
    QMap<QString, RoomInvite>
                invite; ///< The rooms that the user has been invited to
    QVariantMap leave; ///< The rooms that the user has left or been banned from
};

/**
 * @brief Base class for matrix events
 *
 */
class PUBLIC Event : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(Event)

  public:
    using MatrixObj::MatrixObj;

    /**
     * @brief Various types of Event
     *
     */
    enum Type {
        // Presence events
        M_PRESENCE,

        // Room events
        M_ROOM_MEMBER,
        M_ROOM_MESSAGE,
        M_ROOM_NAME,
        M_ROOM_CREATE,

        // Ephemeral events
        M_TYPING,

        // Other types of events
        M_UNKNOWN,
        M_OTHER
    };

    Type        type;    ///< Type of this event
    QVariantMap content; ///< The event content
};

/**
 * @brief Optional extra information about an Event
 *
 */
class PUBLIC UnsignedData : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(UnsignedData)

  public:
    using MatrixObj::MatrixObj;

    /**
     * @brief The time in milliseconds that has elapsed since the event was sent
     *
     */
    int age;

    /**
     * @brief Optional. The event that redacted this event, if any
     *
     */
    Event redactedBecause;

    /**
     * @brief The client-supplied transaction ID, if the client being given
       the event is the same one which sent it.
     *
     */
    QString transactionId;
};

/**
 * @brief Content for an Event
 *
 */
class PUBLIC EventContent : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(EventContent)

  public:
    using MatrixObj::MatrixObj;

    enum Membership {
        MEMBERSHIP_INVITE,
        MEMBERSHIP_JOIN,
        MEMBERSHIP_KNOCK,
        MEMBERSHIP_LEAVE,
        MEMBERSHIP_BAN,
        MEMBERSHIP_UNKNOWN,
    };

    QString    avatarUrl;   ///< The avatar URL for this user, if any
    QString    displayName; ///< The display name for this user, if any
    Membership membership;  ///< Required. The membership state of the user

    /**
     * @brief Flag indicating if the room containing this event was created with
       the intention of being a direct chat
     *
     */
    bool isDirect;

    /**
     * @brief Contains optional extra information about the event
     *
     */
    UnsignedData unsignedData;
};

/**
 * @brief Content for an event of type m.room.create
 *
 */
class PUBLIC CreateContent : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(CreateContent)

  public:
    QString creator;      ///< Required. The user_id of the Room creator
    bool federate = true; ///< Whether users on other servers can join this Room
    QString roomVersion = "1"; ///< The version of the Room

    // Predecessor fields (optional)
    QString roomId;  ///< The ID of the old room
    QString eventId; ///< The event ID of the last known event in the old room
};

/**
 * @brief Content for an event of type m.room.name
 *
 */
class PUBLIC RoomNameContent : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(RoomNameContent)

  public:
    QString name;
};

/**
 * @brief Represents a room event
 *
 */
class PUBLIC RoomEvent : public Event {
    CLASS_CONSTRUCTOR(RoomEvent, Event)

  public:
    QString eventId; ///< Required. The globally unique event identifier

    /**
     * @brief Required. Contains the fully-qualified ID of the user who sent
       this event
     *
     */
    QString sender;

    /**
     * @brief Required. Timestamp in milliseconds on originating homeserver when
       this event was sent
     *
     */
    int serverTs;

    /**
     * @brief Contains optional extra information about the event
     *
     */
    UnsignedData unsignedData;
};

/**
 * @brief Represents state of a Room
 *
 */
class PUBLIC StateEvent : public RoomEvent {
    CLASS_CONSTRUCTOR(StateEvent, RoomEvent)

  public:
    using RoomEvent::RoomEvent;

    /**
     * @brief Optional. The previous content for this event. If there is no
      a previous content, this key will be missing
     *
     */
    EventContent prevContent;

    /**
     * @brief Required. A unique key which defines the overwriting semantics for
       this piece of room state
     *
     */
    QString stateKey;
};

/**
 * @brief Same as StateEvent, but with less info
 *
 */
class PUBLIC StrippedStateEvent : public Event {
    CLASS_CONSTRUCTOR(StrippedStateEvent, Event)

  public:
    EventContent content;  ///< Required. The content for the event
    QString      stateKey; ///< Required. The state_key for the event
    QString      sender;   ///< Required. The sender for the event
};

/**
 * @brief Represents a matrix user, used by Room to store members
 *
 */
class PUBLIC User : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new User object
     *
     * @param room The Room this user is
     * @param userId
     * @param displayName
     * @param avatarUrl
     */
    User(Room *         room,
         const QString &userId,
         const QString &displayName = "",
         const QString &avatarUrl   = "");

    QString userId;      ///< Fully qualified matrix user ID
    QString displayName; ///< User display name, if any
    QString avatarUrl;   ///< User avatar URL, if any
};
} // namespace Types
} // namespace MatrixCpp

// Include other classes
#include <MatrixCpp/Room.hpp>