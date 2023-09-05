
#ifndef TRAACT_CORE_EXPORT_H
#define TRAACT_CORE_EXPORT_H

#ifdef TRAACT_CORE_STATIC_DEFINE
#  define TRAACT_CORE_EXPORT
#  define TRAACT_CORE_NO_EXPORT
#else
#  ifndef TRAACT_CORE_EXPORT
#    ifdef traact_core_EXPORTS
        /* We are building this library */
#      define TRAACT_CORE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define TRAACT_CORE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef TRAACT_CORE_NO_EXPORT
#    define TRAACT_CORE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef TRAACT_CORE_DEPRECATED
#  define TRAACT_CORE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TRAACT_CORE_DEPRECATED_EXPORT
#  define TRAACT_CORE_DEPRECATED_EXPORT TRAACT_CORE_EXPORT TRAACT_CORE_DEPRECATED
#endif

#ifndef TRAACT_CORE_DEPRECATED_NO_EXPORT
#  define TRAACT_CORE_DEPRECATED_NO_EXPORT TRAACT_CORE_NO_EXPORT TRAACT_CORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TRAACT_CORE_NO_DEPRECATED
#    define TRAACT_CORE_NO_DEPRECATED
#  endif
#endif

#endif /* TRAACT_CORE_EXPORT_H */
