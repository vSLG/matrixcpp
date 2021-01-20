#pragma once

#include <QtCore/QtGlobal>

#if defined(MatrixCpp_EXPORTS)
#define PUBLIC Q_DECL_EXPORT
#else
#define PUBLIC Q_DECL_IMPORT
#endif