
#ifndef TRAACT_EXPORT_H
#define TRAACT_EXPORT_H

#ifdef TRAACT_STATIC_DEFINE
#  define TRAACT_EXPORT
#  define TRAACT_NO_EXPORT
#else
#  ifndef TRAACT_EXPORT
#    ifdef traact_core_EXPORTS
        /* We are building this library */
#      define TRAACT_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define TRAACT_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef TRAACT_NO_EXPORT
#    define TRAACT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef TRAACT_DEPRECATED
#  define TRAACT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TRAACT_DEPRECATED_EXPORT
#  define TRAACT_DEPRECATED_EXPORT TRAACT_EXPORT TRAACT_DEPRECATED
#endif

#ifndef TRAACT_DEPRECATED_NO_EXPORT
#  define TRAACT_DEPRECATED_NO_EXPORT TRAACT_NO_EXPORT TRAACT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TRAACT_NO_DEPRECATED
#    define TRAACT_NO_DEPRECATED
#  endif
#endif

#endif /* TRAACT_EXPORT_H */
