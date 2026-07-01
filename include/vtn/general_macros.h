#pragma once
#ifndef GENERAL_MACROS_H
#define GENERAL_MACROS_H

#include <stdbool.h>        /* for true/false                    */
#include <stddef.h>         /* for size_t                        */
#include <stdint.h>         /* for int32_t                       */
#include <stdlib.h>         /* for abort,random,free             */
#include <stdio.h>          /* for FILE                          */
#include <assert.h>         /* for assert                        */
#include <sys/types.h>      /* for ssize_t                       */

#if(defined(__linux__) || defined(__ANDROID__))
#include <sys/cdefs.h>      /* for __BEGIN_DECLS / __END_DECLS   */
#endif // __linux__ || __ANDROID__

#ifdef __ANDROID__
#include <android/log.h>    /* for log msg on logcat             */
#endif // __ANDROID__

#ifdef _WIN32
#include <crtdbg.h>         /* for _ASSERT_AND_INVOKE_WATSON     */
#include <sal.h>            /* for annotation: mark parameters   */
#ifndef __func__
#define __func__ __FUNCTION__
#endif // !__func__
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__ 
#endif // !__PRETTY_FUNCTION__
#else
typedef void*               HANDLE;
typedef void*               PVOID;
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
#endif // _WIN32

#ifdef __cplusplus
#define _THE_NULL nullptr /* NULL keyword for cpp */ 
#else
#define _THE_NULL NULL    /* NULL keyword for c   */ 
#endif // __cplusplus

#ifndef UNUSED
#define UNUSED(x)           (void)(x)
#endif // !UNUSED
#ifndef UNUSED_ATTR
#ifdef _WIN32
#define UNUSED_ATTR 
#else
#define UNUSED_ATTR         __attribute__((unused))
#endif // _WIN32
#endif // !UNUSED_ATTR

// widely useful macros. pay attention to the influence of double computation!
#ifndef __max
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !__max
#ifndef __min
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !__min
#ifndef __abs
#define __abs(x)   ((x) >= 0 ? (x) : -(x))  
#endif // !__abs

#ifndef FREE
#define FREE(ptr) do{ if(ptr) { free(ptr); (ptr) = _THE_NULL; } }while(0)
#endif // !FREE

#ifndef ARRAY_LEN
#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))
#endif // !ARRAY_LEN
#ifndef INVALID_FD
#define INVALID_FD (-1)
#endif // !INVALID_FD
#ifndef CONCAT
#define CONCAT(a, b) a##b
#endif // !CONCAT
#ifndef STRINGIFY
#define STRINGIFY(a) #a
#endif // !STRINGIFY
#ifndef NULLABLE_STRING
#define NULLABLE_STRING(a) ((a) ? (a) : "(null)")
#endif // !NULLABLE_STRING

#if (defined(Sleep) || defined(usleep))
// nothing need define
#elif (defined(PLATFORM_HENGXUAN) || defined(CMSIS_OS_VER))
#include "cmsis_os2.h"
#define Sleep(ms) osDelay(ms)
#define usleep(micro_seconds) Sleep((micro_seconds) / 1000)
#define sleep(seconds) Sleep((seconds) * 1000)

#elif (defined(_WIN32))
#pragma warning(push)
#pragma warning(disable : 5105)

#include <windows.h>

#ifndef usleep
#define usleep(micro_seconds) Sleep((micro_seconds) / 1000)
#endif // !usleep
#ifndef sleep
#define sleep(seconds) Sleep((seconds) * 1000)
#endif // !sleep

#pragma warning(pop)

#else /* for unix */

#include <unistd.h>
#ifndef Sleep
#define Sleep(millseconds) usleep((millseconds) * 1000)
#endif // !Sleep

#endif // Sleep

#ifndef ARRAY_LEN
#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))
#endif // !ARRAY_LEN


#ifdef _WIN32
// to make MSC happy
#define stricmp(s1, s2) _stricmp(s1, s2)
#define strnicmp(s1, s2, n) _strnicmp(s1, s2, n)
// stricmp is win32 function. suggest to use strcasecmp for cross platform
#define strcasecmp(s1, s2) stricmp(s1, s2)
#define strncasecmp(s1, s2, n) strnicmp(s1, s2, n)
#ifndef strdup
#define strdup(s) _strdup(s)
#endif			// !strdup
#ifndef strndup // windows not implement strndup, let's do it.
	char *strndup(const char *s, size_t n);
#endif // !strndup

/**
 * to make MSC happy
 * origin strcpy_s(dest, destSize, src)
 * here we assume that "dest" is big enough to storage "src", so we use 'strlen(src) + 1' as destSize.
 * be careful!
 */
#define strcpy(dest, src) strcpy_s((dest), strlen(src) + 1U, (src))
#define strncpy(dest, src, cp_count) strncpy_s((dest), (cp_count) + 1U, (src), (cp_count))
#define strcat(dest, src) strcat_s((dest), strlen(dest) + strlen(src) + 1U, (src))
#define strtok_r(str, delimiter, ctx) strtok_s((str), (delimiter), (ctx))
	// #define snprintf(buf, buf_size, format, ...)  _snprintf_s(buf, buf_size, (buf_size) - 1, format, ## __VA_ARGS__)
#else
// stricmp is windows function. suggest to use strcasecmp for cross platform
#define stricmp(s1, s2) strcasecmp(s1, s2)
#define strnicmp(s1, s2, n) strncasecmp(s1, s2, n)
#endif // _WIN32



#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

//just to make MSC happy
#define access(path_name, mode)   _access(path_name, mode)
#define mkdir(path, mode)         _mkdir(path)
#define read(fd, buf, count)      _read(fd, buf, count)
#define write(fd, buf, count)     _write(fd, buf, count)

static inline FILE* _fopen_safe(char const* _FileName, char const* _Mode)
{
	FILE* _ftemp = _THE_NULL;
	fopen_s(&_ftemp, _FileName, _Mode);
	return _ftemp;
}
//to make MSC happy
#define fopen(file_name, mode) _fopen_safe(file_name, mode)
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32
#define fclose(fp) do{if(fp){ fclose(fp); (fp) = _THE_NULL; }}while(0)

#endif // !GENERAL_MACROS_H
