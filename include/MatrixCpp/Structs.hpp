/**
 * @file Structs.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Declares many Matrix-related data structures
 * @version 0.1
 * @date 2021-01-25
 *
 * Copyright (c) 2021 vslg
 *
 */

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

namespace MatrixCpp::Structs {

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
    virtual void parseData() = 0;

    bool m_broken = false;
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

        // Ephemeral events
        M_TYPING,

        // Other types of events
        M_UNKNOWN,
        M_OTHER
    };

    Type type; ///< Type of this event
};

/**
 * @brief Optional extra information about an Event
 *
 */
class PUBLIC UnsignedData : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(UnsignedData)

  public:
    using MatrixObj::MatrixObj;

    int     age;
    Event   redactedBecause;
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

    QString      avatarUrl;
    QString      displayName;
    QString      membership;
    bool         isDirect;
    UnsignedData unsignedData;
};

/**
 * @brief Represents a room event
 *
 */
class PUBLIC RoomEvent : public Event {
    CLASS_CONSTRUCTOR(RoomEvent, Event)

  public:
    QVariantMap  content;
    QString      eventId;
    QString      sender;
    int          serverTs;
    UnsignedData unsignedData;
};

/**
 * @brief Represents state of a Room
 *
 */
class PUBLIC StateEvent : public RoomEvent {
    CLASS_CONSTRUCTOR(StateEvent, RoomEvent)

  public:
    EventContent prevContent;
    QString      stateKey;
};

/**
 * @brief Same as StateEvent, but with less info
 *
 */
class PUBLIC StrippedStateEvent : public Event {
    CLASS_CONSTRUCTOR(StrippedStateEvent, Event)

  public:
    EventContent content;
    QString      stateKey;
    QString      sender;
};

/**
 * @brief A matrix Room object
 *
 */
class PUBLIC Room : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(Room)

  public:
    QVariantMap       summary;
    QList<StateEvent> state;
    QVariantMap       timeline;
    QList<Event>      ephemeral;
    QList<Event>      accountData;
    QVariantMap       unreadNotifications;
};

/**
 * @brief A Room which user has been invited to
 *
 */
class PUBLIC RoomInvite : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(RoomInvite)

  public:
    QList<StrippedStateEvent> events;
};

class PUBLIC Rooms : public MatrixObj {
    MATRIXOBJ_CONSTRUCTOR(Rooms)

  public:
    QMap<QString, Room>       join;
    QMap<QString, RoomInvite> invite;
    QVariantMap               leave;
};
} // namespace MatrixCpp::Structs