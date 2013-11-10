#ifndef FTNOIR_PROTOCOL_BASE_GLOBAL_H
#define FTNOIR_PROTOCOL_BASE_GLOBAL_H

#include <QtGlobal>

#ifndef OPENTRACK_MAIN
# if !defined(_MSC_VER)
#   define FTNOIR_PROTOCOL_BASE_EXPORT __attribute__ ((visibility ("default")))
# else
#   define FTNOIR_PROTOCOL_BASE_EXPORT Q_DECL_EXPORT
#endif
#else
# define FTNOIR_PROTOCOL_BASE_EXPORT Q_DECL_IMPORT
#endif

#endif // FTNOIR_PROTOCOL_BASE_GLOBAL_H
