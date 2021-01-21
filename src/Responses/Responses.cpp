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
    // Start by checking if root data is a Map
    if ((QMetaType::Type) data.type() != QMetaType::QVariantMap) {
        qCritical() << "VersionsResponse: expected data to be a Map";
        this->m_broken = true;
        return;
    }

    QVariantMap dataMap = data.toMap();

    if (dataMap["versions"].isNull()) {
        qCritical() << "VersionsResponse: data does not contain 'versions' key";
        this->m_broken = true;
    } else
        this->versions = dataMap["versions"].value<QStringList>();

    QVariantMap unstableFeaturesMap = dataMap["unstable_features"].toMap();

    if (!unstableFeaturesMap.isEmpty()) {
        QVariantMap::iterator it = unstableFeaturesMap.begin();
        for (; it != unstableFeaturesMap.end(); ++it)
            this->unstableFeatures[it.key()] = it.value().toBool();
    }
}