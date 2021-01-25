/**
 * @file Responses.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Declares possible matrix responses
 * @version 0.1
 * @date 2021-01-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QNetworkReply>

#include <MatrixCpp/Structs.hpp>
#include <MatrixCpp/export.hpp>

/**
 * @brief Generates basic header for subclasses of Response
 *
 */
#define RESPONSE_CONSTRUCTOR(type)                         \
  protected:                                               \
    virtual void parseData() override;                     \
                                                           \
  public:                                                  \
    type(QByteArray rawResponse) : Response(rawResponse) { \
        if (!this->isError() && !this->isBroken())         \
            this->parseData();                             \
    };                                                     \
    type(QVariant data) : Response(data) {                 \
        if (!this->isError() && !this->isBroken())         \
            this->parseData();                             \
    };                                                     \
    type(const Response &other) : Response(other) {        \
        if (!this->isError() && !this->isBroken())         \
            this->parseData();                             \
    };

/**
 * @brief Checks if data is a Map and create dataMap
 *
 */
#define CHECK_MAP()                           \
    QVariantMap dataMap = this->data.toMap(); \
    BROKEN(dataMap.isEmpty())

namespace MatrixCpp::Responses {

/**
 * @brief A matrix server response
 *
 */
class PUBLIC Response : public Structs::MatrixObj {
  public:
    // Inherit constructors
    using Structs::MatrixObj::MatrixObj;

    /**
     * @brief Tell if Response is an error
     *
     * @return true
     * @return false
     */
    bool isError() const;

  protected:
    virtual void parseData() override;
};

/**
 * @brief Response yet to be fetched
 *
 */
class PUBLIC ResponseFuture : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new ResponseFuture object
     *
     * @param reply The ongoing NetworkReply
     */
    explicit ResponseFuture(QNetworkReply *reply);

    /**
     * @brief Construct a new ResponseFuture object
     *
     * @param other
     */
    ResponseFuture(const ResponseFuture &other);

    /**
     * @brief Get the Response object when request finishes
     *
     * @return Response
     */
    Response result();

    /**
     * @brief Get the Response object when request finishes
     *
     * @tparam T The type of Response to return
     * @return T The Response object
     */
    template <class T> T result();

  signals:
    /**
     * @brief Is fired when we have response fetched and parsed
     *
     */
    void responseComplete(Response response);

  private:
    /**
     * @brief Initializes object. Takes same params of default constructor
     *
     * @param reply
     */
    void init(QNetworkReply *reply);

    QNetworkReply *m_reply;
    bool           m_finished = false;
    QByteArray     m_rawResponse;
};

class PUBLIC ErrorResponse : public Response {};

/**
 * @brief Response object for server version
 *
 */
class PUBLIC VersionsResponse : public Response {
    RESPONSE_CONSTRUCTOR(VersionsResponse)

  public:
    QMap<QString, bool>
        unstableFeatures; ///< Experimental features the server supports.
    QStringList versions; ///< Required. The supported versions.
};

/**
 * @brief Response object for login types
 *
 */
class PUBLIC LoginTypesResponse : public Response {
    RESPONSE_CONSTRUCTOR(LoginTypesResponse)

  public:
    QList<QMap<QString, QString>>
                flows; ///< The homeserver's supported login types (raw)
    QStringList types; ///< The homeserver's supported login types
};

/**
 * @brief Response object for login
 *
 */
class PUBLIC LoginResponse : public Response {
    RESPONSE_CONSTRUCTOR(LoginResponse)

  public:
    QString userId;      ///< Fully qualified user ID returned by server
    QString accessToken; ///< Access token for using with auth-required requests
    QString deviceId;    ///< Device ID returned by server
    QUrl    homeserver;  ///< Well known information (see WellKnownResponse)
    QUrl    identityServer; ///< Well known information (see WellKnownResponse)
};

/**
 * @brief Response object for well-known server info
 *
 */
class PUBLIC WellKnownResponse : public Response {
    RESPONSE_CONSTRUCTOR(WellKnownResponse)

  public:
    /**
     * @brief Homeserver URL returned by server and to be used for subsequent
       requests
     *
     */
    QUrl homeserver;
    QUrl identityServer; ///< Identity server URL returned by server
};

class PUBLIC SyncResponse : public Response {
    RESPONSE_CONSTRUCTOR(SyncResponse)

  public:
    /**
     * @brief Required. String to be used as since param of next sync
     *
     */
    QString nextBatch;

    QVariantMap rooms; ///< Updates to Rooms

    /**
     * @brief The updates to the presence status of other users
     *
     */
    QList<Structs::Event> presence;

    /**
     * @brief The global private data created by this user
     *
     */
    QList<Structs::Event> accountData;

    /**
     * @brief Information on the send-to-device messages for the client device
     *
     */
    QList<Structs::Event> toDevice;

    QVariantMap deviceLists; ///< Information on end-to-end device updates
};
} // namespace MatrixCpp::Responses