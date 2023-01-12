#ifndef EXPORT_H_
#define EXPORT_H_

// EXPORT
#if defined(__GNUC__)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

// INLINE
#define INLINE static inline

// DEPRECATED
#if defined(NO_DEPRECATED)
#define DEPRECATED
#elif defined(__GNUC__) || defined(__clang__)
#define DEPRECATED __attribute__((deprecated))
#else
#define DEPRECATED
#endif

// EV_UNUSED
#if defined(__GNUC__)
#define UNUSED __attribute__((visibility("unused")))
#else
#define UNUSED
#endif

// @param[IN | OUT | INOUT]
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

// @field[OPTIONAL | REQUIRED | REPEATED]
#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef REQUIRED
#define REQUIRED
#endif

#ifndef REPEATED
#define REPEATED
#endif

#define EXTERN_C extern
#define BEGIN_EXTERN_C
#define END_EXTERN_C

#define BEGIN_NAMESPACE(ns)
#define END_NAMESPACE(ns)
#define USING_NAMESPACE(ns)

#ifndef DEFAULT
#define DEFAULT(x)
#endif

#ifndef ENUM
#define ENUM(e)       \
    typedef enum e e; \
    enum e
#endif

#ifndef STRUCT
#define STRUCT(s)       \
    typedef struct s s; \
    struct s
#endif

#endif  // EV_EXPORT_H_
