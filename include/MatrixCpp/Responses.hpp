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
     * @brief Tell if Response is broken (e.g. malformed or empty json)
     *
     * @return true
     * @return false
     */
    bool isBroken();

    /**
     * @brief Return response as another type of response
     *
     * @tparam T The desided Response type
     * @return T The new Response
     */
    template <class T> T as();

    /**
     * @brief Get the JSON-formatted data
     *
     * @return QByteArray
     */
    QByteArray getJson();

    QVariant data;

  private:
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
    // template <class T> T getResponse();

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

// class PUBLIC ErrorResponse : public Response {};
} // namespace MatrixCpp::Responses