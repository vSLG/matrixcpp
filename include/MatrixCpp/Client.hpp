/**
 * @file Client.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Declares Client class
 * @version 0.1
 * @date 2021-01-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

#include "export.hpp"
#include <MatrixCpp/Responses.hpp>

namespace MatrixCpp {
class PUBLIC Client : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new Client object
     *
     * @param host Homeserver host
     * @param parent QObject parent, if any
     */
    explicit Client(const QString &host, QObject *parent = nullptr);

    /**
     * @brief Construct a new Client object
     *
     * @param homeserver Homeserver URL for this client
     * @param parent QObject parent, if any
     */
    explicit Client(const QUrl &homeserverUrl, QObject *parent = nullptr);

    // Enums and structs

    /**
     * @brief Available presence states for a client
     *
     */
    enum Presence { PRESENCE_ONLINE, PRESENCE_BUSY, PRESENCE_OFFLINE };

    // API calls

    /**
     * @brief Get the Server Version and unstable features
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture getServerVersion() const;

    /**
     * @brief Get server supported login types
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture getLoginTypes() const;

    // Public variables

    QUrl homeserverUrl;

  signals:
    void abortRequests();

  private:
    /**
     * @brief HTTP get request to specified path on homeserver
     *
     * @param path
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture get(QString path) const;

    /**
     * @brief HTTP get request to specified URL
     *
     * @param url
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture get(QUrl url) const;

    QNetworkAccessManager *m_nam;
};
} // namespace MatrixCpp