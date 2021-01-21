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

#include "export.hpp"

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
        this->parseData();                                 \
    };                                                     \
    type(QVariant data) : Response(data) {                 \
        this->parseData();                                 \
    };                                                     \
    type(const Response &other) : Response(other) {        \
        this->parseData();                                 \
    };

namespace MatrixCpp::Responses {

/**
 * @brief A matrix server response
 *
 */
class PUBLIC Response {
  public:
    /**
     * @brief Construct a new Response object
     *
     * @param rawResponse Raw response string
     */
    explicit Response(QByteArray rawResponse);

    /**
     * @brief Construct a new Response object
     *
     * @param data
     */
    Response(QVariant data);

    /**
     * @brief Construct a new Response object
     *
     * @param other
     */
    Response(const Response &other);

    /**
     * @brief Tell if Response is broken (e.g. malformed or empty json)
     *
     * @return true
     * @return false
     */
    bool isBroken() const;

    /**
     * @brief Return response as another type of response
     *
     * @tparam T The desided Response type
     * @return T The new Response
     */
    template <class T> T as() const;

    /**
     * @brief Get the JSON-formatted data
     *
     * @return QByteArray
     */
    QByteArray getJson() const;

    QVariant data;

  protected:
    virtual void parseData();

    bool m_broken = false;
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

    /**
     * @brief Is fired when a network error occurs
     *
     */
    void errorOccurred(QString errorString);

  private:
    bool       m_finished = false;
    QByteArray m_rawResponse;
};

class PUBLIC ErrorResponse : public Response {};

/**
 * @brief Response object for server version
 *
 */
class PUBLIC VersionsResponse : public Response {
    RESPONSE_CONSTRUCTOR(VersionsResponse)

  public:
    QMap<QString, bool> unstableFeatures;
    QStringList         versions;
};

/**
 * @brief Response object for login types
 *
 */
class PUBLIC LoginTypesResponse : public Response {
    RESPONSE_CONSTRUCTOR(LoginTypesResponse)

  public:
    QList<QMap<QString, QString>> flows;
    QStringList                   types;
};
} // namespace MatrixCpp::Responses