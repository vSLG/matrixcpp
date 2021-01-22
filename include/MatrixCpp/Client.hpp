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
     * @param homeserverUrl Homeserver URL for this client
     * @param parent QObject parent, if any
     */
    explicit Client(const QUrl &homeserverUrl, QObject *parent = nullptr);

    /**
     * @brief (sync) Request for well_known and update the client
     *
     */
    void loadDiscovery();

    // Enums and structs

    /**
     * @brief Available presence states for a client
     *
     */
    enum Presence { PRESENCE_ONLINE, PRESENCE_BUSY, PRESENCE_OFFLINE };

    // API calls

    /**
     * @brief (async) Get server discovery information. This will also update
       the client
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture getDiscovery();

    /**
     * @brief (async) Get the Server Version and unstable features
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture getServerVersion() const;

    /**
     * @brief (async) Get server supported login types
     *
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture getLoginTypes() const;

    /**
     * @brief (async) Logs in to the specified account with password
     *
     * @param user User name, can be fully qualified or local userId
     * @param password Password for the user
     * @param deviceId Optional deviceId
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture
    login(QString user, QString password, QString deviceId = "");

    // Getters & setters

    /**
     * @brief Gets the user_id
     *
     * @return QString
     */
    QString userId() const;

    /**
     * @brief Gets the device_id
     *
     * @return QString
     */
    QString deviceId() const;

    /**
     * @brief Gets the access_token
     *
     * @return QString
     */
    QString accessToken() const;

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

    /**
     * @brief Sends POST JSON to specified path
     *
     * @param path
     * @param data The data to be sent. Will be JSON encoded
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture send(QString path, QVariant data) const;

    /**
     * @brief Sends POST JSON to specified URL
     *
     * @param url
     * @param data The data to be sent. Will be JSON encoded
     * @return Responses::ResponseFuture
     */
    Responses::ResponseFuture send(QUrl url, QVariant data) const;

    QNetworkAccessManager *m_nam;

    QString m_userId;
    QString m_accessToken;
    QString m_deviceId;
};
} // namespace MatrixCpp