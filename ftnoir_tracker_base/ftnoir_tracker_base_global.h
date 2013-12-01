#ifndef FTNOIR_TRACKER_BASE_GLOBAL_H
#define FTNOIR_TRACKER_BASE_GLOBAL_H

#include <QtGlobal>

#ifndef FTNOIR_TRACKER_BASE_EXPORT
#   ifndef OPENTRACK_MAIN
#    if !defined(_MSC_VER)
#      define FTNOIR_TRACKER_BASE_EXPORT __attribute__ ((visibility ("default")))
#    else
#     define FTNOIR_TRACKER_BASE_EXPORT Q_DECL_EXPORT
#   endif
#   else
#       define FTNOIR_TRACKER_BASE_EXPORT Q_DECL_IMPORT
#   endif
#endif

#endif // FTNOIR_TRACKER_BASE_GLOBAL_H
