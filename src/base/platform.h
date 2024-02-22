#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "config.h"

// OS
#if defined(linux) || defined(__linux) || defined(__linux__)
#define OS_LINUX
#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#include <TargetConditionals.h>
#define OS_MAC
#define OS_DARWIN
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define OS_FREEBSD
#define OS_BSD
#elif defined(__NetBSD__)
#define OS_NETBSD
#define OS_BSD
#elif defined(__OpenBSD__)
#define OS_OPENBSD
#define OS_BSD
#else
#warning "Untested operating system platform!"
#endif

#define OS_UNIX

// ARCH
#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
#define ARCH_X64
#define ARCH_X86_64
#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define ARCH_X86
#define ARCH_X86_32
#elif defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)
#define ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
#define ARCH_ARM
#elif defined(__mips64__)
#define ARCH_MIPS64
#elif defined(__mips__)
#define ARCH_MIPS
#else
#warning "Untested hardware architecture!"
#endif

// COMPILER
#if defined(__GNUC__)
#define COMPILER_GCC

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#elif defined(__clang__)
#define COMPILER_CLANG

#elif defined(__MINGW32__) || defined(__MINGW64__)
#define COMPILER_MINGW

#elif defined(__MSYS__)
#define COMPILER_MSYS

#elif defined(__CYGWIN__)
#define COMPILER_CYGWIN

#else
#warning "Untested compiler!"
#endif

// headers
#ifdef OS_SAT
// TODO
#else
#include <dirent.h> // for mkdir,rmdir,chdir,getcwd
#include <unistd.h>

// socket
#include <arpa/inet.h>
#include <netdb.h> // for gethostbyname
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/select.h>
#include <sys/socket.h>

#define ev_sleep(s)   sleep(s)
#define ev_msleep(ms) usleep((ms) * 1000)
#define ev_usleep(us) usleep(us)
#define ev_delay(ms)  ev_msleep(ms)
#define ev_mkdir(dir) mkdir(dir, 0777)
#endif

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef void* HANDLE;
#include <strings.h>
#define stricmp  strcasecmp
#define strnicmp strncasecmp

// ENDIAN
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif
#ifndef NET_ENDIAN
#define NET_ENDIAN BIG_ENDIAN
#endif

// BYTE_ORDER
#ifndef BYTE_ORDER
#if defined(__BYTE_ORDER)
#define BYTE_ORDER __BYTE_ORDER
#elif defined(__BYTE_ORDER__)
#define BYTE_ORDER __BYTE_ORDER__
#elif defined(ARCH_X86) || defined(ARCH_X86_64) || defined(__ARMEL__) || defined(__AARCH64EL__) || \
    defined(__MIPSEL) || defined(__MIPS64EL)
#define BYTE_ORDER LITTLE_ENDIAN
#elif defined(__ARMEB__) || defined(__AARCH64EB__) || defined(__MIPSEB) || defined(__MIPS64EB)
#define BYTE_ORDER BIG_ENDIAN
#elif defined(OS_WIN)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#warning "Unknown byte order!"
#endif
#endif

// ANSI C
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __cplusplus
#if HAVE_STDBOOL_H
#include <stdbool.h>
#else
#ifndef bool
#define bool char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif
#endif
#endif

#if HAVE_STDINT_H
#include <stdint.h>
#endif

typedef float float32_t;
typedef double float64_t;

typedef int (*method_t)(void* userdata);
// typedef void (*procedure_t)(void* userdata);

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h> // for gettimeofday
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#endif // PLATFORM_H_
