//
// Created by hj on 2021/12/28.
//

#ifndef VADSDK_SYSTEM_INFO_H
#define VADSDK_SYSTEM_INFO_H

#if defined(_MSC_VER) /* Microsoft Visual C++ */
#if !defined(VADAPI)
#define VADAPI     __stdcall
#define VADAPITYPE __stdcall
#endif
#pragma pack(push, 8)
#else /* Any other including Unix */
#if !defined(VADAPI)
#define VADAPI __attribute__((visibility("default")))
#define VADAPITYPE
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

VADAPI const char *getMac();

#ifdef __cplusplus
}
#endif

#endif //VADSDK_SYSTEM_INFO_H
