[English](./README.md) | 简体中文

# 功能介绍

`speech_agent_audio` 是智能语音 agent 的前端音频处理模块，负责麦克风阵列音频采集、降噪、唤醒词检测、命令词识别和 VAD（语音活动检测）。适用于 D-Robotics RDK 配套的麦克风阵列（支持 4mic 和 6mic 两种类型）。

应用场景：智能语音前端处理，为下游 ASR 和 TTS 模块提供降噪后的音频流和唤醒/命令词事件，主要应用于智能家居、智能座舱、智能穿戴设备等领域。

## 处理流程

```
麦克风阵列 (8ch, 16kHz, S16_LE)
  → ALSA 采集 (MicphoneGetThread)
  → 通道重映射 (8ch→6ch, 仅 4mic 模式)
  → VTN 引擎 (libvtn.so, 降噪 + 唤醒)
  → VTN 回调分发:
      ├─ 唤醒词事件 → /wkp_text
      ├─ 命令词结果 → /audio_smart
      ├─ DOA 角度 → /doa_data
      ├─ ASR 音频 (降噪后) → /asr_data
      └─ VAD 音频 → VAD 引擎 (libevad.so) → /vad_data
```

# 使用方法

## 准备工作

- RDK 已烧录好 Ubuntu 系统镜像
- 音频板正确连接到 RDK 的麦克风阵列接口
- 麦克风类型为 4mic 或 6mic

## 运行方式

### 1. 下载资源包

- 编译时会自动下载资源包，如果想手动下载资源包，可以使用下面命令将资源包下载到项目根目录，再进行编译

```bash
wget https://archive.d-robotics.cc/TogetheROS/files/speech_solution/speech_agent_audio/speech_agent_audio_v1.0.0.tar.gz
```

### 2. 命令词配置

命令词定义在 `/opt/tros/humble/lib/speech_agent_audio/res/cmd_word/cmd_word.json` 文件中，默认为：

```json
{
    "cmd_word": [
        {
            "text": "向前走",
            "pinyin": "xiang4 qian2 zou3"
        },
        {
            "text": "向后退",
            "pinyin": "xiang4 hou4 tui4"
        },
        {
            "text": "向左转",
            "pinyin": "xiang4 zuo3 zhuan3"
        },
        {
            "text": "向右转",
            "pinyin": "xiang4 you4 zhuan3"
        },
        {
            "text": "停止运动",
            "pinyin": "ting2 zhi3 yun4 dong4"
        }
    ]
}
```

`text` 为命令词文本，`pinyin` 为对应的拼音（数字表示声调）。用户可根据需要自行配置，推荐使用中文，词语长度 3~5 个字。

### 3. 编译

```bash
source /opt/tros/humble/setup.bash
colcon build --packages-select speech_agent_audio
```

### 4. 启动

```bash
# 4mic(默认)
ros2 launch speech_agent_audio speech_agent_audio.launch.py

# 6mic
ros2 launch speech_agent_audio speech_agent_audio.launch.py mic_type:=6mic

# 同时启动audio、asr和tts, 脚本使用示例
ros2 launch speech_agent_audio speech_agent.launch.py asr_pub_topic:=/prompt_text 
```

### 5. 结果验证

启动后终端输出如下信息表示音频设备初始化成功：

```text
[WARN] [speech_agent_audio]: Parameter:
 micphone_name: plughw:0,1
 mic_type: 4mic
 ...
[WARN] [speech_agent_audio]: speech engine init success
[WARN] [speech_agent_audio]: vad init
[WARN] [speech_agent_audio]: start to capture audio
```

说唤醒词后，可观察到唤醒事件发布：

```text
[WARN] [speech_agent_audio]: wakeup word:你好你好
```

说命令词后，可观察到命令词识别结果：

```text
[WARN] [speech_agent_audio]: cmd word:向前走
```

# 接口说明

## 话题

| 名称 | 消息类型 | 说明 |
| --- | --- | --- |
| /wkp_text | std_msgs/msg/String | 唤醒词文本 |
| /audio_smart | audio_msg/msg/SmartAudioData | 命令词识别结果 |
| /asr_data | audio_msg/msg/SmartAudioData | 降噪后 ASR 音频数据 |
| /vad_data | audio_msg/msg/SmartAudioData | VAD 状态事件 (BOS/MID/EOS/超时) |
| /doa_data | audio_msg/msg/SmartAudioData | 声源定位角度 |

## 参数

| 参数名 | 类型 | 说明 | 是否必须 | 支持的配置 | 默认值 |
| --- | --- | --- | --- | --- | --- |
| micphone_name | string | 音频采集设备名 | 否 | 根据实际情况配置 | plughw:0,1 |
| mic_type | string | 麦克风类型 | 否 | 4mic, 6mic | 4mic |
| wkp_event_pub_topic_name | string | 唤醒事件发布话题 | 否 | 根据实际情况配置 | /wkp_text |
| doa_pub_topic_name | string | DOA 角度发布话题 | 否 | 根据实际情况配置 | /doa_data |
| cmd_pub_topic_name | string | 命令词结果发布话题 | 否 | 根据实际情况配置 | /audio_smart |
| asr_pub_topic_name | string | ASR 音频数据发布话题 | 否 | 根据实际情况配置 | /asr_data |
| vad_pub_topic_name | string | VAD 数据发布话题 | 否 | 根据实际情况配置 | /vad_data |
| push_wakeup | int | 是否推送唤醒词前缀 | 否 | 0, 1 | 0 |
| vad_eos_timeout | int | VAD 尾点超时时间 (ms) | 否 | 根据实际情况配置 | 500 |

## 运行依赖

- ALSA (libasound) — 音频采集
- VTN SDK (libvtn.so) — 降噪和唤醒引擎
- VAD SDK (libevad.so) — 语音活动检测
- speech_agent_bootstrap — SDK 初始化封装库
- jsoncpp — 命令词配置解析

# 常见问题

1. 无法打开音频设备？
   - 确认音频板连接是否正常
   - 确认 `micphone_name` 参数配置正确（通常为 `plughw:0,1`）
   - 使用 `arecord -l` 查看可用音频设备

2. 4mic 和 6mic 有什么区别？
   - 4mic：使用线性4mic音频板采集 8 通道音频，重映射为 6 通道后送入 VTN 引擎
   - 6mic：使用环形6mic音频板采集 8 通道音频，直接送入 VTN 引擎（不做通道重映射）

3. 唤醒词不生效？
   - 确认 VTN 引擎初始化成功（日志中应有 "speech engine init success"）
   - 确认 vtn.ini 配置文件存在