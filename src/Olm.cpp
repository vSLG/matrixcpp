/**
 * @file Olm.cpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Implements Olm
 * @version 0.1
 * @date 2021-02-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#include "Olm.hpp"

using namespace MatrixCpp;
using namespace MatrixCpp::Crypto;

Olm::Olm(Client *client) : QObject(client) {
    this->m_account = olm_account(malloc(olm_account_size()));
}

Olm::~Olm() {
    olm_clear_account(this->m_account);
}