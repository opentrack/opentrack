#pragma once

#ifdef BUILD_migration
#   ifdef _WIN32
#       define OPENTRACK_MIGRATION_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_MIGRATION_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_MIGRATION_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_MIGRATION_LINKAGE
#   else
#       define OPENTRACK_MIGRATION_EXPORT OPENTRACK_MIGRATION_LINKAGE
#   endif

#else
    #ifdef _WIN32
    #    define OPENTRACK_MIGRATION_LINKAGE __declspec(dllimport)
    #else
    #    define OPENTRACK_MIGRATION_LINKAGE
    #endif

    #ifndef _MSC_VER
    #    define OPENTRACK_MIGRATION_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_MIGRATION_LINKAGE
    #else
    #    define OPENTRACK_MIGRATION_EXPORT OPENTRACK_MIGRATION_LINKAGE
    #endif
#endif
