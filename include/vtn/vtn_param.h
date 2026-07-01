#pragma once
#ifndef VTN_PARAM_H
#define VTN_PARAM_H

#include <stddef.h>

// ================================= ATTR MACRO START =================================

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(ATTR_HIDE_SYMBOLS) && !defined(ATTR_IMPORT_SYMBOLS) && !defined(ATTR_EXPORT_SYMBOLS)
#define ATTR_EXPORT_SYMBOLS
#endif

#ifdef __WINDOWS__
#define _ATTR_CDECL __cdecl
#define _ATTR_STDCALL __stdcall
#define ATTR_CALLING_CONVENTION _ATTR_CDECL

#if defined(ATTR_HIDE_SYMBOLS)
#define ATTR_PUBLIC(type) type ATTR_CALLING_CONVENTION
#elif defined(ATTR_EXPORT_SYMBOLS)
#define ATTR_PUBLIC(type) __declspec(dllexport) type ATTR_CALLING_CONVENTION
#elif defined(ATTR_IMPORT_SYMBOLS)
#define ATTR_PUBLIC(type) __declspec(dllimport) type ATTR_CALLING_CONVENTION
#endif

#else /* !__WINDOWS__ */

#define _ATTR_CDECL
#define _ATTR_STDCALL
#define ATTR_CALLING_CONVENTION _ATTR_CDECL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && (defined(ATTR_EXPORT_SYMBOLS))
#define ATTR_PUBLIC(type) __attribute__((visibility("default"))) type
#else
#define ATTR_PUBLIC(type) type
#endif

#endif // __WINDOWS__

// ================================= ATTR MACRO END  =================================

/**
 * @brief protocol version. In order to check for mismatched usage of header files and libraries.
 *       if protocol changed, this number will increased by developer of this project.
 *       user should NOT changed it manually!!!
 */
#define VTN_PARAM_PROTOCOL_VER 0

/**
 * @brief base number of vtn_interact_type_e.
 *        user should NOT changed it manually!!!
 */
#define MODULE_VTN_TYPE_BASE 100

// ================================= KEYWORD TYPE START =================================
// main keyword
#define VTN_KEYWORD_TYPE_STR_MAIN "main"
// command(cmd)(asr) keyword
#define VTN_KEYWORD_TYPE_STR_CMD "cmd"
// alias of macro "VTN_KEYWORD_TYPE_STR_CMD"
#define VTN_KEYWORD_TYPE_STR_COMMAND VTN_KEYWORD_TYPE_STR_CMD
// ================================= KEYWORD TYPE END   =================================

// ============================================ VTN_PARAM_ACTION START ============================================
#define VTN_PARAM_ACTION_SET_LOG_INFO "SET_LOG_INFO"               // <-- set log info: level...
#define VTN_PARAM_ACTION_SET_THREAD_AFFINITY "SET_THREAD_AFFINITY" // <-- bind cpu core to thread
#define VTN_PARAM_ACTION_SET_THREAD_PRIORITY "SET_THREAD_PRIORITY" // <-- set thread priority
#define VTN_PARAM_ACTION_SET_KEY_VALUES "SET_KEY_VALUES"           // <-- set key_values parameter
#define VTN_PARAM_ACTION_RELOAD_MODULE "RELOAD_MODULE"             // <-- reload internal engine module
// ============================================ VTN_PARAM_ACTION END   ============================================

/* status code for vtn function */ // <-- vtn 函数状态码.
typedef enum
{
    /* everything is all right */ // <-- 一切正常.
    VTN_STATUS_SUCCESS = 0,
    /* general fail */ // <-- 通用失败, 请结合上下文 ERROR 级别日志判断原因.
    VTN_STATUS_GENERAL_FAIL,
    /* init failed */ // <-- 初始化失败.
    VTN_STATUS_INIT_FAILED,
    /* no authorization, contact your business partner */ // <-- 无授权, 请联系您的业务合作方.
    VTN_STATUS_NO_AUTHORIZATION,
    /* param invalid, check your param */ // <-- 参数无效, 请检查您的参数.
    VTN_STATUS_INVALID_PARAM,
    /* param value invalid, check your param value */ // <-- 参数值无效, 请检查您的参数值.
    VTN_STATUS_INVALID_PARAM_VALUE,
    /* illegal state, check your call flow */ // <-- 非法状态, 请检查您的调用流程.
    VTN_STATUS_ILLEGAL_STATE,
    /* buffer full, you can try again later */ // <-- 缓冲区已满, 请稍后重试.
    VTN_STATUS_BUFFER_FULL,
    /* buffer not enough, should expand it */ // <-- 缓冲区不足, 请扩展缓冲区(具体指的是应扩展 vtn_interact_info_t.out的 str 和 raw内存大小).
    VTN_STATUS_BUFFER_NOT_ENOUGH,
    /* action not support */ // <-- 不支持的操作.
    VTN_STATUS_NOT_SUPPORT,

    /* count for vtn_status */ // <-- vtn_status 枚举值数量.
    VTN_STATUS_COUNT,
} vtn_status_code_e;

/*
 * callback type for vtn.
 *
 * notice for Android java developer: should sync it with VtnApiJni.java
 */
typedef enum
{
    // reserved type. in general, it won't triggered. // <-- 保留事件, 一般不会触发.
    VTN_CALLBACK_TYPE_RESERVED = 0,
    // thread state: for hook thread state. must be first one.  // <-- 线程状态改变事件
    VTN_CALLBACK_TYPE_THREAD_STATE,

    //  general param event. // <-- 通用带参下发事件。
    //    in most scenario, we callback event type specified below.
    VTN_CALLBACK_TYPE_PARAM_EVENT,

    // for auth result // <-- 授权结果
    VTN_CALLBACK_TYPE_AUTHORIZATION = 6,

    // for wakeup event  // <-- 唤醒事件, 比 WAKEUP_DETAIL 事件触发的要早一些, 只有 关键词/得分 这些基础信息, 常用于显示UI, 播放提示音.
    VTN_CALLBACK_TYPE_WAKEUP = 10,
    // wakeup_detail event // <-- 唤醒详情, 与 WAKEUP 事件区别在于这个回调事件带 波束/角度/能量 信息(前提是麦阵方案支持), 如果你不关心这些额外信息则可以忽略此事件.
    VTN_CALLBACK_TYPE_WAKEUP_DETAIL,
    // for closed loop event (also called logpcm) // <-- 闭环音频收集 事件
    VTN_CALLBACK_TYPE_CLOSED_LOOP,
    // segment audio of before wakeup(about 1.5s). // <-- 唤醒词前端点往前推 1.5s 音频 事件
    VTN_CALLBACK_TYPE_BEFORE_WAKE_SEGMENT,
    // segment audio of after wakeup(wake_end until to current). // <-- 唤醒词尾端点到当前时刻音频 事件
    VTN_CALLBACK_TYPE_AFTER_WAKE_SEGMENT,

    // for ivw // <-- 做 唤醒 用的内部音频, 开发者一般不需要关注.
    VTN_CALLBACK_TYPE_AUDIO_IVW0 = 20,
    VTN_CALLBACK_TYPE_AUDIO_IVW1,
    VTN_CALLBACK_TYPE_AUDIO_IVW2,
    VTN_CALLBACK_TYPE_AUDIO_IVW3,
    // for ivw e2e noise ref. // <-- 做 唤醒 用的内部音频, 开发者一般不需要关注.
    VTN_CALLBACK_TYPE_AUDIO_IVW_REF0 = 25,
    VTN_CALLBACK_TYPE_AUDIO_IVW_REF1,
    VTN_CALLBACK_TYPE_AUDIO_IVW_REF2,
    VTN_CALLBACK_TYPE_AUDIO_IVW_REF3,

    // for recognition: denoised audio, also called iat audio. // <-- 去回声降噪后的音频, 常用于做 ASR 识别. 一般在唤醒后输出.
    VTN_CALLBACK_TYPE_AUDIO_REC = 30,
    // for multi rec: noise ref.  // <-- 识别音频对应的噪声参考. 若方案不支持, 则不会抛出此音频.
    VTN_CALLBACK_TYPE_AUDIO_REC_NOISE,
    // for vad. // <-- VAD 音频, 强抑制, 常用于做VAD端点检测. 窄波束方案建议使用此路音频做 ASR 识别. 若方案不支持输出VAD音频, 则不会抛出此音频.
    VTN_CALLBACK_TYPE_AUDIO_VAD,
    // for vcall. vcall engine maybe outputted via AUDIO_REC. // <-- 视频通话音频, 只有降噪+通话的组合方案才会通过此通道输出通话音频, 常规通话方案的输出是通过 AUDIO_REC 通道.
    VTN_CALLBACK_TYPE_AUDIO_VCALL,
    // for hearing. // <-- 听感音频. 若方案不支持, 则不会抛出此音频.
    VTN_CALLBACK_TYPE_AUDIO_HEARING,

    // for AEC audio. // <-- AEC 音频. 若方案不支持输出, 则不会抛出此音频.
    VTN_CALLBACK_TYPE_AUDIO_AEC0 = 40,
    VTN_CALLBACK_TYPE_AUDIO_AEC1,

} vtn_callback_type_e;

/* callback data for vtn_api */ // <-- vtn 回调数据.
typedef struct
{
    // callback type // <-- 回调事件类型
    vtn_callback_type_e type;
    // body data. in general, this is string. // <-- 回调数据主体, 一般是字符串. 根据回调类型存放的数据类型有所不同, 比如 type == AUIDO_XXX 则是音频数据.
    const void *data;
    // size of data // <-- 回调主体数据字节大小. 如果是字符串, 则是strlen(data) + 1, 也就是包含 '\0' 结束符的字符串长度; 若是音频数据, 则是音频数据字节大小.
    int data_size;
    // extra param // <-- 回调数据额外参数, 补充此次回调事件的额外信息. 具体使用方式参见demo. 常规没有需求可忽略.
    const void *param;
    // size of param // <-- 回调数据额外参数字节大小.
    int param_size;
} vtn_callback_data_t;

/* callback pack for vtn_api */
typedef struct
{
    /* user_data that will be passed to handler function param. */ // <-- 用户数据指针. 在回调时会通过 user_data 参数原样带出.
    void *user_data;
    /* function prototype of handle callback */ // <-- vtn回调函数指针.
    int(ATTR_CALLING_CONVENTION *handler)(const vtn_callback_data_t *data_p, void *user_data);
} vtn_callback_pack_t;

/* param for vtn_api_init */
typedef struct
{
    // [in] version of protocol. should be VTN_PARAM_PROTOCOL_VER. do NOT hardcode it by the specified number. // <-- 协议版本. 必须是 VTN_PARAM_PROTOCOL_VER. 不要硬编码为其他数字.
    int protocol_ver;

    // [in] user callback pack. // <-- vtn回调函数指针 和 用户数据指针. 必须提供.
    vtn_callback_pack_t callback;

    // [in/out]
    struct
    {
        // [in] in params. // <-- in 参数. 必须提供, 通常是个 JSON 字符串.
        //      such as: { "params":{ "config_uri":"./res/vtn/vtn.ini", "sn":"abcdefgh12345678" } }
        const char *in;
        // [in] memory size of in. should include '\0' if it's string. // <-- in 参数内存字节大小. 若 in 是字符串, 则必须包含 '\0' 结束符.
        size_t in_size;

        // [in/out] out params: output engine info. // <-- out 参数: 用户需提供内存用于引擎输出引擎信息等. 若不需要, 则可以设为 NULL.
        //          caller need provide memory. can be NULL if you don't care.
        //          example: after init, you may get json like: { "params":{ "frame_info":{"step_frame":"1280", "step_frame_ms":"80"} } }
        char *out;
        // [in/out] memory size of out. // <-- out 参数内存字节大小.
        size_t out_size;
    } params;
} vtn_init_param_t;

/* vtn_api interactive type */
typedef enum
{
    // reserved. // <-- 保留类型. 一般不需要关注.
    VTN_INTERACT_TYPE_RESERVED = 0,

    //  general param action. // <-- 常规参数交互.
    //  the supported parameters can be found in the "VTN_PARAM_ACTION_XXX" above.
    //    generally speaking, no need to use this parameter,
    //    please use the parameters specified below.
    VTN_INTERACT_TYPE_PARAM_ACTION = MODULE_VTN_TYPE_BASE,

    // chip_auth: get hint for unlock key // <-- 芯片授权: 获取解锁密钥提示信息
    VTN_INTERACT_TYPE_AUTH_CHIP_GET_HINT = MODULE_VTN_TYPE_BASE + 10,
    // chip_auth: unlock key by hint. // <-- 芯片授权: 通过解锁密钥提示信息解锁.
    VTN_INTERACT_TYPE_AUTH_CHIP_SET_KEY,

    // get queue unhandled byte size: which indicate the left data size in queue. // <-- 获取队列中未处理数据的字节大小.
    VTN_INTERACT_TYPE_GET_UNHANDLED_DATA_SIZE = MODULE_VTN_TYPE_BASE + 20,
    // get output audio group. // <-- 获取当前输出音频组.
    VTN_INTERACT_TYPE_GET_OUTPUT_AUDIO_GROUP,
    // set output audio group. // <-- 设置当前输出音频组.
    VTN_INTERACT_TYPE_SET_OUTPUT_AUDIO_GROUP,
    // set fixed channel for rec audio. // <-- 设置识别音频的固定通道索引. 一般不需要关注.
    VTN_INTERACT_TYPE_SET_FIXED_CHANNEL_FOR_REC,

    // evaluate wake word // <-- 评估自定义唤醒词级别.
    VTN_INTERACT_TYPE_EVALUATE_WAKE_WORD = MODULE_VTN_TYPE_BASE + 30,
    // generate wake word resource. // <-- 生成自定义唤醒词资源.
    VTN_INTERACT_TYPE_GENERATE_WAKE_WORD_RES,
    // add wake word resource. // <-- 添加自定义唤醒词资源.
    VTN_INTERACT_TYPE_ADD_WAKE_WORD_RES,
    // remove wake word resource. // <-- 删除自定义唤醒词资源.
    VTN_INTERACT_TYPE_REMOVE_WAKE_WORD_RES,

    // feed audio. // <-- 输入音频. 一般是 麦克风+参考信号 的交叉音频.
    VTN_INTERACT_TYPE_FEED_AUDIO = MODULE_VTN_TYPE_BASE + 40,
    // set beam: enable output recognition audio for the beam.
    //          in most scenario, no need set it manually, we will setup the right beam after wakeup.
    // <-- 设置波束. 用于在唤醒后, 开启指定波束的输出识别音频. 一般不需要关注, 引擎会在唤醒后自动设置正确的波束.
    VTN_INTERACT_TYPE_SET_BEAM,
    // switch wake mode. // <-- 切换唤醒模式( main 或 cmd).
    VTN_INTERACT_TYPE_SWITCH_WAKE_MODE,

} vtn_interact_type_e;

/* interact params for vtn_interact_info_t */
typedef struct
{
    // size of str. (include string NULL terminator). // <-- 字符串大小(需包含 '\0' 结束符).
    size_t str_size;
    // string pointer // <-- 字符串指针.
    char *str;

    // size of raw // <-- raw内存字节大小.
    size_t raw_size;
    // raw data pointer // <-- raw内存数据指针.
    void *raw;
} vtn_interact_param_t;

/**
 * @brief output type of interact result,
 *        indicate the type of out.str or out.raw.
 * 
 *        用于指示 vtn_interact_info_t.with_out 成员的输出类型.
 *        VTN_INTERACT_OUTPUT_TYPE_NONE: 无输出.
 *        VTN_INTERACT_OUTPUT_TYPE_STR: 输出了字符串信息. 可以读取 out.str 和 out.str_size.
 *        VTN_INTERACT_OUTPUT_TYPE_RAW: 输出raw数据信息. 可以读取 out.raw 和 out.raw_size.
 */
typedef enum
{
    VTN_INTERACT_OUTPUT_TYPE_NONE = 0,
    VTN_INTERACT_OUTPUT_TYPE_STR = 1,
    VTN_INTERACT_OUTPUT_TYPE_RAW = (1 << 1),
} vtn_interact_output_type_e;

/**
 * @brief interact info for vtn
 *        vtn 交互参数结构体: 包含交互类型、输入参数、输出参数等信息.
 *
 * @note
 * 这里设计成 in/out 参数, 并用 string + raw 参数搭配的这种组合 原因如下：
 *   0. 标准化输入输出: 所有交互都通过这种模式规范化输入输出.
 *   1. 兼容 Android jni 层操作: 方便 jni 层透传输入输出数据, 无需再做接口业务数据转换和对接,
 *      否则对于每种 interact_type 还需额外的 jni 对接开发工作量.
 *   2. 方便后面需求扩展: string + raw 这种参数组合基本上可以覆盖绝大数交互场景的需求，
 *      新增需求无需改动接口, 只需对 string 或者 raw 内容做扩展即可.
 *      str: 一般放交互参数信息, 一般来说是个 json string.
 *      raw: 一般配合 str 作为补充, 比如写音频这个交互类型, 音频就放在 raw 参数里指向的内存.
 *
 * 缺点: 开发者得知道每种交互类型的输入输出是什么，按照协议组装、解析交互数据.
 *       不过话说回来, 开发者本来就得理解每种交互的输入/输出是什么.
 *
 * FAQ:
 *  Q:  为什么不把交互类型(vtn_interact_type_e type) 放到 in.str 参数里的 json 里作为一个字段呢? 额外放一个成员显得突兀?
 *  A:  是为了优化性能: 高频交互操作是写音频, 若把交互类型放到 str 里势必会导致每次都需去解析 json 获取 type 字段, 导致性能下降.
 *
 *  Q:  为什么交互输出参数 (vtn_interact_param_t out) 的内存也是由调用者分配, 调用者该分配多少呢?
 *  A:  0. 调用者分配内存方案可以复用交互参数及内存，另外引擎内部不好管理输出的内存，不知道调用者何时不需要这块内存，
 *         所以最终决定由调用者分配内存，引擎负责将输出的结果拷贝到输出参数(out)的内存里. 另外其实带参数交互的情况也不多.
 *      1. 具体分配多少内存这个没有个规定值, 得看具体的交互类型，通常建议 4KB 就足够了, 大多带交互输出的是 json string 形式.
 *         另外即使调用者分配的内存不够, 接口也会抛出对应的错误码和日志信息, 可以根据错误码来扩大交互输出参数的内存.
 *      2. 还有一点提醒下开发者: 交互参数 in/out 里的 str.size 和 raw.size 都是字节大小, 告诉引擎的大小不能比实际分配的大,
 *         否则可能会导致引擎访问你提供的这块内存越界, 造成不可挽回的非法访问等内存崩溃问题. 这一点一定要注意.
 *
 *  Q:  结构体 vtn_interact_info_t.with_out 成员参数有什么作用?
 *  A:  结构体成员 with_out 在交互前(调用 interact 接口前)应该置为 0(VTN_INTERACT_OUTPUT_TYPE_NONE),
 *      这个参数会在引擎交互接口中被赋值, 用于指示当前交互的输出的 out结构体成员 结果包含哪些信息, 可以用 位与 的方式判断输出结果的数据类型.
 *      0. 若 (0 == with_out) , 则代表此次交互没有任何内容输出, 那么开发者不用解析.
 *      1. 若 (0 != (with_out & VTN_INTERACT_OUTPUT_TYPE_STR)) , 则代表 out.str 有字符串可以被读取, 数据大小为 out.str_size (字节大小, 包含'\0'结束符).
 *      2. 若 (0 != (with_out & VTN_INTERACT_OUTPUT_TYPE_RAW)) , 则代表 out.raw 有数据可以被读取, 数据大小为 out.raw_size (字节大小).
 *     至于这个 out.str 和 out.raw 里面代表的信息内容, 根据不同的交互类型有不同的含义, 具体的参见示例 sample 代码.
 *
 */
typedef struct
{
    // [in] interact type  // <-- 交互类型
    vtn_interact_type_e type;

    // [in] interact param. // <-- 交互入参信息
    vtn_interact_param_t in;

    /* [out] indicates whether there is an output.
             see type on vtn_interact_output_type_e.
             notice: maybe multi-type data will be outputted.
       // <-- 交互出参类型. 可能会输出多种类型数据. 参见 vtn_interact_output_type_e.
    */
    int with_out;

    /* [out] result of interact.
            notice: caller must provide memory.
       // <-- 交互出参信息. 调用者需在调用时提供内存信息(str内存 和 raw内存).
    */
    vtn_interact_param_t out;
} vtn_interact_info_t;

// ================================= macro helper for assemble vtn_init_param_t START =================================
// macro helper for define vtn_init_param_t. // <-- 定义 vtn_init_param_t 变量.
#define DEFINE_ENGINE_INIT_PARAM_INFO(callback_handler, callback_user_data) \
    char _buf_init_in[1024] = {0};                                          \
    char _buf_init_out[2048] = {0};                                         \
    vtn_init_param_t init_param;                                            \
    memset(&init_param, 0, sizeof(init_param));                             \
    init_param.protocol_ver = VTN_PARAM_PROTOCOL_VER;                       \
    init_param.callback.handler = callback_handler;                         \
    init_param.callback.user_data = (void *)(callback_user_data);           \
    init_param.params.in = &(_buf_init_in[0]);                              \
    init_param.params.in_size = 0;                                          \
    init_param.params.out = &(_buf_init_out[0]);                            \
    init_param.params.out_size = sizeof(_buf_init_out);

// ================================= macro helper for assemble vtn_init_param_t END =================================

// ================================= macro helper for init interact_info START =================================

// macro helper for define interact_info variable by interact type. // <-- 根据交互类型定义交互信息变量.
#define DEFINE_INTERACT_INFO_BY_TYPE(IN_TYPE, IN_STR_SIZE, IN_RAW_SIZE, OUT_STR_SIZE, OUT_RAW_SIZE) \
    vtn_interact_info_t interact_info;                                                              \
    memset(&interact_info, 0, sizeof(interact_info));                                               \
    interact_info.type = IN_TYPE;                                                                   \
    char _buf_interact_str_in[IN_STR_SIZE] = {0};                                                   \
    char _buf_interact_raw_in[IN_RAW_SIZE] = {0};                                                   \
    char _buf_interact_str_out[OUT_STR_SIZE] = {0};                                                 \
    char _buf_interact_raw_out[OUT_RAW_SIZE] = {0};                                                 \
    interact_info.in.str = &(_buf_interact_str_in[0]);                                              \
    interact_info.in.str_size = sizeof(_buf_interact_str_in);                                       \
    interact_info.in.raw = &(_buf_interact_raw_in[0]);                                              \
    interact_info.in.raw_size = sizeof(_buf_interact_raw_out);                                      \
    interact_info.out.str = &(_buf_interact_str_out[0]);                                            \
    interact_info.out.str_size = sizeof(_buf_interact_str_out);                                     \
    interact_info.out.raw = &(_buf_interact_raw_out[0]);                                            \
    interact_info.out.raw_size = sizeof(_buf_interact_raw_out);                                     \
    interact_info.with_out = VTN_INTERACT_OUTPUT_TYPE_NONE;

// macro helper for define interact_info variable. // <-- 使用默认交互类型定义交互信息变量.
#define DEFINE_INTERACT_INFO(IN_STR_SIZE, IN_RAW_SIZE, OUT_STR_SIZE, OUT_RAW_SIZE) \
    DEFINE_INTERACT_INFO_BY_TYPE(VTN_INTERACT_TYPE_RESERVED, IN_STR_SIZE, IN_RAW_SIZE, OUT_STR_SIZE, OUT_RAW_SIZE)

// macro helper for reset interact_info variable. reset for reuse it. // <-- 根据交互类型重置交互信息变量(重置 str 和 raw 内存).
#define RESET_INTERACT_INFO_BY_TYPE(IN_TYPE)                        \
    do                                                              \
    {                                                               \
        _buf_interact_str_in[0] = '\0';                             \
        _buf_interact_raw_in[0] = 0;                                \
        _buf_interact_str_out[0] = '\0';                            \
        _buf_interact_raw_out[0] = 0;                               \
        interact_info.type = IN_TYPE;                               \
        interact_info.in.str = &(_buf_interact_str_in[0]);          \
        interact_info.in.str_size = sizeof(_buf_interact_str_in);   \
        interact_info.in.raw = &(_buf_interact_raw_in[0]);          \
        interact_info.in.raw_size = sizeof(_buf_interact_raw_out);  \
        interact_info.out.str = &(_buf_interact_str_out[0]);        \
        interact_info.out.str_size = sizeof(_buf_interact_str_out); \
        interact_info.out.raw = &(_buf_interact_raw_out[0]);        \
        interact_info.out.raw_size = sizeof(_buf_interact_raw_out); \
        interact_info.with_out = VTN_INTERACT_OUTPUT_TYPE_NONE;     \
    } while (0)

// macro helper for reset interact_info variable. reset for reuse it. // <-- 使用默认交互类型重置交互信息变量(重置 str 和 raw 内存)..
#define RESET_INTERACT_INFO() RESET_INTERACT_INFO_BY_TYPE(VTN_INTERACT_TYPE_RESERVED)

// macro helper for define interact_info. only define str memory, not include raw. // <-- 根据交互类型定义交互信息变量(仅定义字符串内存，不包括 raw 内存).
#define DEFINE_STR_INTERACT_INFO_BY_TYPE(IN_TYPE, IN_STR_SIZE, OUT_STR_SIZE) \
    vtn_interact_info_t interact_info;                                       \
    memset(&interact_info, 0, sizeof(interact_info));                        \
    interact_info.type = IN_TYPE;                                            \
    char _buf_interact_str_in[IN_STR_SIZE] = {0};                            \
    char _buf_interact_str_out[OUT_STR_SIZE] = {0};                          \
    interact_info.in.str = &(_buf_interact_str_in[0]);                       \
    interact_info.in.str_size = sizeof(_buf_interact_str_in);                \
    interact_info.out.str = &(_buf_interact_str_out[0]);                     \
    interact_info.out.str_size = sizeof(_buf_interact_str_out);              \
    interact_info.with_out = VTN_INTERACT_OUTPUT_TYPE_NONE;

// macro helper for define interact_info. only define str memory, not include raw. // <-- 使用默认交互类型定义交互信息变量(仅定义字符串内存，不包括 raw 内存).
#define DEFINE_STR_INTERACT_INFO(IN_STR_SIZE, OUT_STR_SIZE) \
    DEFINE_STR_INTERACT_INFO_BY_TYPE(VTN_INTERACT_TYPE_RESERVED, IN_STR_SIZE, OUT_STR_SIZE)

// macro helper for reset interact_info.
// because of we reuse this interact_info, so we need reset it before interact with different types. // <-- 根据交互类型重置交互信息变量(仅重置字符串内存，不包括 raw 内存).
#define RESET_STR_INTERACT_INFO_BY_TYPE(IN_TYPE)                    \
    do                                                              \
    {                                                               \
        _buf_interact_str_in[0] = '\0';                             \
        _buf_interact_str_out[0] = '\0';                            \
        interact_info.type = IN_TYPE;                               \
        interact_info.in.str = &(_buf_interact_str_in[0]);          \
        interact_info.in.str_size = sizeof(_buf_interact_str_in);   \
        interact_info.out.str = &(_buf_interact_str_out[0]);        \
        interact_info.out.str_size = sizeof(_buf_interact_str_out); \
        interact_info.with_out = VTN_INTERACT_OUTPUT_TYPE_NONE;     \
    } while (0)

// macro helper for reset interact_info.
// because of we reuse this interact_info, so we need reset it before interact with different types. // <-- 根据交互类型重置交互信息变量(仅重置字符串内存，不包括 raw 内存).
#define RESET_STR_INTERACT_INFO() RESET_STR_INTERACT_INFO_BY_TYPE(VTN_INTERACT_TYPE_RESERVED)

// ================================= macro helper for init interact_info END   =================================

#endif // !VTN_PARAM_H
