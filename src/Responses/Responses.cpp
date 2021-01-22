/**
 * @file Responses.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implements various response types
 * @version 0.1
 * @date 2021-01-21
 *
 * Copyright (c) 2021 vslg
 *
 */

#include <MatrixCpp/Responses.hpp>

using namespace MatrixCpp::Responses;

/*
 * VersionsResponse
 */

void VersionsResponse::parseData() {
    QVariantMap dataMap = this->data.toMap();

    // Start by checking if root data is a Map
    if (dataMap.isEmpty()) {
        qCritical() << "VersionsResponse: expected data to be a Map";
        this->m_broken = true;
        return;
    }

    if (dataMap["versions"].isNull()) {
        qCritical() << "VersionsResponse: data does not contain 'versions' key";
        this->m_broken = true;
        return;
    }

    this->versions = dataMap["versions"].value<QStringList>();

    QVariantMap unstableFeaturesMap = dataMap["unstable_features"].toMap();

    if (!unstableFeaturesMap.isEmpty()) {
        QVariantMap::iterator it = unstableFeaturesMap.begin();
        for (; it != unstableFeaturesMap.end(); ++it)
            this->unstableFeatures[it.key()] = it.value().toBool();
    }
}

/*
 * LoginTypesResponse
 */

void LoginTypesResponse::parseData() {
    QVariantMap dataMap = this->data.toMap();

    // Start by checking if root data is a Map
    if (dataMap.isEmpty()) {
        qCritical() << "LoginTypesResponse: expected data to be a Map";
        this->m_broken = true;
        return;
    }

    if (dataMap["flows"].isNull()) {
        qCritical() << "LoginTypesResponse: data does not contain 'flows' key";
        this->m_broken = true;
        return;
    }

    QVariantList flows = dataMap["flows"].toList();

    for (QVariant item : flows) {
        QVariantMap entry = item.toMap();

        if (entry.isEmpty() || entry["type"].isNull())
            continue;

        QMap<QString, QString> newEntry;
        newEntry["type"] = entry["type"].toString();

        this->types.append(entry["type"].toString());
        this->flows.append(newEntry);
    }
}

/*
 * LoginResponse
 */

void LoginResponse::parseData() {
    QVariantMap dataMap = this->data.toMap();

    // Start by checking if root data is a Map
    if (dataMap.isEmpty()) {
        qCritical() << "LoginResponse: expected data to be a Map";
        this->m_broken = true;
        return;
    }

    for (QString key : {"user_id", "access_token", "device_id"})
        if (dataMap[key].isNull()) {
            qCritical() << "LoginResponse: data does not contain" << key
                        << "key";
            this->m_broken = true;
            return;
        }

    this->userId      = dataMap["user_id"].toString();
    this->accessToken = dataMap["access_token"].toString();
    this->deviceId    = dataMap["device_id"].toString();

    if (dataMap["well_known"].isNull())
        return; // End it here

    this->homeserver = dataMap["well_known"]
                           .toMap()["m.homeserver"]
                           .toMap()["base_url"]
                           .toUrl();

    this->identityServer = dataMap["well_known"]
                               .toMap()["m.identity_server"]
                               .toMap()["base_url"]
                               .toUrl();

    if (this->homeserver.isEmpty())
        qWarning(
            "LoginResponse: well_known present, but no m.homeserver present");
}