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
 * @brief Generates basic header for subclasses of Response
 *
 */
#define MATRIXOBJ_CONSTRUCTOR(type)                         \
  protected:                                                \
    virtual void parseData() override;                      \
                                                            \
  public:                                                   \
    type(QByteArray rawResponse) : MatrixObj(rawResponse) { \
        if (!this->isBroken())                              \
            this->parseData();                              \
    };                                                      \
    type(QVariant data) : MatrixObj(data) {                 \
        if (!this->isBroken())                              \
            this->parseData();                              \
    };                                                      \
    type(const MatrixObj &other) : MatrixObj(other) {       \
        if (!this->isBroken())                              \
            this->parseData();                              \
    };

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
     * @brief Construct a new MatrixObj object from JSON string
     *
     * @param rawJson The JSON string
     */
    explicit MatrixObj(QByteArray rawJson);

    /**
     * @brief Construct a new MatrixObj object from QVariant data
     *
     * @param data
     */
    MatrixObj(QVariant data);

    /**
     * @brief Construct a new MatrixObj object based on other MatrixObj
     *
     * @param other
     */
    MatrixObj(const MatrixObj &other);

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
} // namespace MatrixCpp::Structs