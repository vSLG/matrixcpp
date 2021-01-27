// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file export.hpp
 * @author vslg (slgf@protonmail.ch)
 * @brief Defines symbol export helpers
 * @version 0.1
 * @date 2021-01-20
 *
 * Copyright (c) 2021 vslg
 *
 */

#pragma once

#include <QtCore/QtGlobal>

#if defined(MatrixCpp_EXPORTS)
#define PUBLIC Q_DECL_EXPORT
#else
#define PUBLIC Q_DECL_IMPORT
#endif