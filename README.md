English | [简体中文](./README_cn.md)

# Introduction

`speech_agent_audio` is the front-end audio processing module of the speech agent system. It handles microphone array audio capture, noise reduction, wake-up word detection, command word recognition, and VAD (Voice Activity Detection). It supports D-Robotics RDK microphone arrays in both 4mic and 6mic configurations.

Use cases: Front-end speech processing that provides noise-reduced audio streams and wake-up/command word events to downstream ASR and TTS modules. Used in smart home, smart cockpit, and wearable devices.

## Pipeline

```
Microphone Array (8ch, 16kHz, S16_LE)
  → ALSA capture (MicphoneGetThread)
  → Channel remapping (8ch→6ch, 4mic mode only)
  → VTN engine (libvtn.so, noise reduction + wake-up)
  → VTN callback dispatches:
      ├─ Wake-up word event → /wkp_text
      ├─ Command word result → /audio_smart
      ├─ DOA angle → /doa_data
      ├─ ASR audio (noise-reduced) → /asr_data
      └─ VAD audio → VAD engine (libevad.so) → /vad_data
```

# Usage Guide

## Preparation

- D-Robotics RDK with Ubuntu system image installed
- Audio board correctly connected to the RDK microphone array interface
- Microphone type: 4mic or 6mic

## Running Audio Front-End Processing

### 1. DownLoad the resource pack

```bash
wget https://archive.d-robotics.cc/TogetheROS/files/speech_solution/speech_agent_audio/speech_agent_audio_v1.0.0.tar.gz
tar -zxvf speech_agent_audio_v1.0.0.tar.gz -C /opt/tros/humble/lib/
```

### 2. Command Word Configuration

Command words are defined in `cmd_word.json`. The default configuration is:

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

`text` is the command word text, and `pinyin` is the corresponding pinyin (numbers indicate tone). Users can customize as needed. Chinese is recommended, with 3-5 characters per word.

### 3. Build

```bash
source /opt/tros/humble/setup.bash
colcon build --packages-select speech_agent_audio
```

### 4. Launch

```bash
# 4mic (default)
ros2 launch speech_agent_audio speech_agent_audio.launch.py

# 6mic
ros2 launch speech_agent_audio speech_agent_audio.launch.py mic_type:=6mic
```

### 5. Verify Results

After startup, the following log output indicates successful audio device initialization:

```text
[WARN] [speech_agent_audio]: Parameter:
 micphone_name: plughw:0,1
 mic_type: 4mic
 ...
[WARN] [speech_agent_audio]: speech engine init success
[WARN] [speech_agent_audio]: vad init
[WARN] [speech_agent_audio]: start to capture audio
```

After speaking the wake-up word, the wake-up event is published:

```text
[WARN] [speech_agent_audio]: wakeup word:你好你好
```

After speaking a command word, the recognition result appears:

```text
[WARN] [speech_agent_audio]: cmd word:向前走
```

# Interface Description

## Topics

| Name | Message Type | Description |
| --- | --- | --- |
| /wkp_text | std_msgs/msg/String | Wake-up word text |
| /audio_smart | audio_msg/msg/SmartAudioData | Command word recognition result |
| /asr_data | audio_msg/msg/SmartAudioData | Noise-reduced audio for downstream ASR |
| /vad_data | audio_msg/msg/SmartAudioData | VAD state events (BOS/MID/EOS/timeout) |
| /doa_data | audio_msg/msg/SmartAudioData | Direction of Arrival angle |

## Parameters

| Parameter | Type | Description | Required | Supported Values | Default |
| --- | --- | --- | --- | --- | --- |
| micphone_name | string | Audio capture device name | No | Per actual hardware | plughw:0,1 |
| mic_type | string | Microphone type | No | 4mic, 6mic | 4mic |
| wkp_event_pub_topic_name | string | Wake-up event topic | No | Per configuration | /wkp_text |
| doa_pub_topic_name | string | DOA angle topic | No | Per configuration | /doa_data |
| cmd_pub_topic_name | string | Command word result topic | No | Per configuration | /audio_smart |
| asr_pub_topic_name | string | ASR audio data topic | No | Per configuration | /asr_data |
| vad_pub_topic_name | string | VAD data topic | No | Per configuration | /vad_data |
| push_wakeup | int | Whether to push wake-up prefix | No | 0, 1 | 0 |
| vad_eos_timeout | int | VAD end-of-speech timeout (ms) | No | Per configuration | 500 |

## Runtime Dependencies

- ALSA (libasound) — audio capture
- VTN SDK (libvtn.so) — noise reduction and wake-up engine
- VAD SDK (libevad.so) — voice activity detection
- speech_agent_bootstrap — SDK initialization wrapper library
- jsoncpp — command word configuration parsing

# Frequently Asked Questions

1. Unable to open the audio device?
   - Check that the audio board is properly connected
   - Verify the `micphone_name` parameter is correct (usually `plughw:0,1`)
   - Use `arecord -l` to list available audio devices

2. What is the difference between 4mic and 6mic?
   - 4mic: Captures 8-channel audio and remaps it to 6 channels before feeding to the VTN engine
   - 6mic: Captures 8-channel audio and feeds it directly to the VTN engine (no channel remapping)

3. Wake-up word not working?
   - Ensure VTN engine initialized successfully (log should show "speech engine init success")
   - Verify authentication credentials are correct
   - Confirm the vtn.ini configuration file exists