/**
 * This file is part of the evad.
 * Copyright © Iflytek Co., Ltd.
 * All Rights Reserved.
 *
 * @author  zrmei
 * @date    2021-12-20
 *
 * @brief
 */
#ifndef IFLYTEK_EVAD_H
#define IFLYTEK_EVAD_H

#if defined(_MSC_VER) /* Microsoft Visual C++ */
#if !defined(VAD_API)
#define VAD_API        __stdcall
#define VAD_DEPRECATED __declspec(deprecated)
#if defined(VAD_LIB_DLL)
#define VAD_EXPORT __declspec(dllexport)
#else
#define VAD_EXPORT
#endif
#endif
#pragma pack(push, 8)
#else /* Any other including Unix */
#if !defined(VAD_API)
#if defined(VAD_LIB_DLL)
#define VAD_API __attribute__((visibility("default")))
#else
#define VAD_API
#endif
#define VAD_DEPRECATED __attribute__((deprecated))
#endif
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *VADHandle;

/**
 * VAD状态定义。
 */
typedef enum {
    VAD_STATUS_FIND_BOS = 0,    // 找到前端点（BOS，Begin of Speech）。
    VAD_STATUS_CONTINUOUS,      // 找到前端点之后，后端点之前的音频都为该状态。
    VAD_STATUS_FIND_EOS,        // 找到后端点（EOS，End of Speech）。
    VAD_STATUS_BOS_TIMEOUT,     // 前端点超时，即一段时间没有找到前端点。
    VAD_STATUS_SPEECH_FINISH,   // 说话结束，即找到后端点之后一段时间没有检测到语音。
    VAD_STATUS_NONE               // 没有状态，即还没有触发上面各状态
} VAD_STATUS;

/**
 * 回调函数。
 *
 * @param status    当前帧状态
 * @param audio     音频数据，需自我复制后使用
 * @param aLen      音频长度
 * @param userData  用户new时传入的数据
 */
typedef void (*vad_audio_cb)(int status, const char *audio, int aLen, void *userData);

/**
 * param说明。
 *   sn             设备唯一ID
 *   appid          用户的 appid，可在 https://aiui.xyfun.cn 平台申请
 *   work_dir       存储临时数据，默认为空，使用临时目录
 *
 *   eos_timeout    设置尾端点超时事件，默认为 1000 单位为 ms
 */
typedef struct
{
    int32_t /*      */ size_of_this; /** 此结构体的大小 sizeof(VADParams) */
    const char /*   */ *param;
    vad_audio_cb /* */ cb;
    void /*         */ *userData;
    int32_t /*      */ res_size;
    const char /*   */ *res_data;
} VADParams;

/**
 *  调用流程
 *
 *     +-------------------+
 *     |     VADNew        |
 *     +---------+---------+
 *               |
 *     +---------v---------+
 *     |   VADSetParamter  |
 *     +---------+---------+
 *               |
 *     +---------v---------+
 *     |      VADAuth      |
 *     +---------+---------+
 *               |
 *     +---------v---------+
 *     |      VADStart     |
 *     +---------+---------+
 *               |
 *     +---------v---------+   +---------------+
 *     |   VADWriteAudio   +---> vad_audio_cb  |
 *     +---------+---------+   +---------------+
 *               |
 *     +---------v---------+
 *     |    VADDestroy     |
 *     +-------------------+
 */

/**
 * 创建VAD实例。
 *
 * @param handle[out] vad句柄
 * @param param 参数设置
 * @return 0表示成功，否则为错误码
 */
int VAD_API VADNew(VADHandle *handle, VADParams param);

/**
 * 启动VAD。
 *
 * @param handle[in] vad句柄
 * @param isSync 是否同步调用，一般传false
 * @return 0表示成功，否则为错误码
 */
int VAD_API VADStart(VADHandle handle, int isSync);

/**
 * 销毁VAD实例。
 *
 * @param handle[in] vad句柄
 * @return 0表示成功，否则为错误码
 */
int VAD_API VADDestroy(VADHandle handle);

/**
 * 鉴权，只有鉴权成功后才能进行下一步操作。
 *
 * @param handle[in] vad句柄
 * @return 0表示成功，否则为错误码
 */
int VAD_API VADAuth(VADHandle handle);

/**
 * 写音频，异步检测VAD。必须写入单通道16k采样16bit编码的pcm音频。
 *
 * @param handle[in] vad句柄
 * @param audio 音频数据
 * @param aLen 数据长度，单位：字节
 * @return 0表示成功，否则为错误码
 */
int VAD_API VADWriteAudio(VADHandle handle, const char *audio, int aLen);

/**
 * 写音频，同步检测VAD。必须写入单通道16k采样16bit编码的pcm音频。
 *
 * 注意：当VADStart传参isSync=true时调用该方法，一般外部开发者用不到该方法。
 *
 * @param handle[in] vad句柄
 * @param audio 音频数据
 * @param aLen 数据长度，单位：字节
 * @return vad状态
 */
int VAD_API VADWrite1280AudioSync(VADHandle handle, const char *audio, int aLen);

/**
 * 音频数据量不够时，强制刷新缓存，默认不需要执行。
 *
 * @param handle[in] vad句柄
 * @return 0表示成功，否则为错误码
 */
int VAD_DEPRECATED VAD_API VADFinishWrite(VADHandle handle);

/**
 * 重置工作状态。
 *
 * @param handle[in] vad句柄
 * @return 0表示成功，否则为错误码
 */
int VAD_API VADReset(VADHandle handle);

/**
 * 设置VAD参数， 需在 VADAuth 之前调用， 之后调用无效。支持的参数设置：
 *
 * bos_timeout：前端点超时时长，即在检测到BOS之前，多长时间没有检测到语音则认为没有说话，单位：ms，默认值：5000。
 * eos_timeout: 后端点超时时长，即在检测到EOS之后，多长时间没有检测到语音就认为说话结束，单位：ms，默认值：1000。
 * max_sentence_length: 一句语音的最大长度，即在检测到BOS之后，最多还能接收多长的有效音频，超过限制后将强制设置EOS，单位：ms，默认值：30000。
 * bos_threshold: 前端点检测门限，值越高越难检测到前端点。取值：0~1，默认值：0.7。
 * eos_threshold: 后端点检测门限，值越高越难检测到后端点。取值：0~1，默认值：0.7。
 *
 * @param handle[in] vad句柄
 * @param key 参数名
 * @param val 参数值
 * @return 0表示成功，否则为错误码
 */
int VAD_API VADSetParameter(VADHandle handle, const char *key, const char *val);

/**
 * 获取版本号。
 *
 * @return 版本号
 */
const char *VAD_API VADVersion();

/**
 * 获取授权错误码描述信息。
 *
 * @param authErr 授权返回的错误码
 * @return 描述信息
 */
const char *VAD_API VADAuthErrorDes(int authErr);

typedef enum {
    VAD_DEBUG,
    VAD_INFO,
    VAD_WARN,
    VAD_ERROR,
    VAD_NONE    // 禁用日志输出
} LogLevel;

/**
 * 设置日志等级。
 *
 * 注意：SDK默认日志等级为VAD_NONE，即不打印日志。
 *
 * @param l 日志等级
 */
void VAD_API VADSetLogLevel(LogLevel l);

#ifdef __cplusplus
}
#endif

#if defined(_MSC_VER) /* Microsoft Visual C++ */
#pragma pack(pop)
#endif

#endif    //IFLYTEK_EVAD_H
