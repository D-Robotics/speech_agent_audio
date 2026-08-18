[简体中文](./README_cn.md) | English

# Overview

`speech_agent_audio` is the front-end audio processing module of the intelligent speech agent, responsible for microphone array audio capture, noise reduction, wake-up word detection, command word recognition, and VAD (Voice Activity Detection). It is designed for the D-Robotics RDK microphone array, supporting both 4-mic and 6-mic configurations.

Use cases: intelligent speech front-end processing, providing noise-reduced audio streams and wake-up/command word events to downstream ASR and TTS modules. It is primarily used in smart home, intelligent cockpit, and smart wearable device applications.

## Processing Pipeline

```
Microphone Array (8ch, 16kHz, S16_LE)
  → ALSA Capture (MicphoneGetThread)
  → Channel Remapping (8ch→6ch, 4-mic mode only)
  → VTN Engine (libvtn.so, noise reduction + wake-up)
  → VTN Callback Dispatch:
      ├─ Wake-up Word Event → /wkp_text
      ├─ Command Word Result → /audio_smart
      ├─ DOA Angle → /doa_data
      ├─ ASR Audio (noise-reduced) → /asr_data
      └─ VAD Audio → VAD Engine (libevad.so) → /vad_data
```

# Getting Started

## Prerequisites

- RDK with Ubuntu system image flashed
- Audio board properly connected to the RDK microphone array interface
- Microphone type: 4-mic or 6-mic

## Running

### 1. Download Resource Package

The resource package is downloaded automatically during compilation. To download it manually, use the following command to download it to the project root directory before compiling:

```bash
wget https://archive.d-robotics.cc/TogetheROS/files/speech_solution/speech_agent_audio/speech_agent_audio_v1.1.0.tar.gz
```

### 2. Command Word Configuration

Command words are defined in `/opt/tros/<TROS_DISTRO>/lib/speech_agent_audio/res/cmd_word/cmd_word.json` (where `<TROS_DISTRO>` is the active TROS distro, e.g. `humble` or `jazzy`, exported by `setup.bash`). The default configuration is:

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

`text` is the command word text, and `pinyin` is the corresponding pinyin (numbers indicate tones). Users can customize the command words as needed. Chinese is recommended, with a word length of 3–5 characters.

### 3. Build

`<TROS_DISTRO>` is the active TROS distribution — `humble` or `jazzy`. Replace it with the distro you have installed and sourced.

```bash
source /opt/tros/<TROS_DISTRO>/setup.bash
colcon build --packages-select speech_agent_audio
```

### 4. Launch

Place resource package in `/opt/tros/${TROS_DISTRO}/lib/`
```bash
cp -r install/speech_agent_audio/lib/speech_agent_audio/ /opt/tros/${TROS_DISTRO}/lib/
```

```bash
# 4mic (default)
ros2 launch speech_agent_audio speech_agent_audio.launch.py

# 6mic
ros2 launch speech_agent_audio speech_agent_audio.launch.py mic_type:=6mic

# Start Audio, ASR, and TTS simultaneously. Script usage examples.
ros2 launch speech_agent_audio speech_agent.launch.py asr_pub_topic:=/prompt_text
```

### 5. Verification

After launching, the following terminal output indicates successful audio device initialization:

```text
[WARN] [speech_agent_audio]: Parameter:
 micphone_name: plughw:0,1
 mic_type: 4mic
 ...
[WARN] [speech_agent_audio]: speech engine init success
[WARN] [speech_agent_audio]: vad init
[WARN] [speech_agent_audio]: start to capture audio
```

After speaking the wake-up word, you should see the wake-up event being published:

```text
[WARN] [speech_agent_audio]: wakeup word:tu3-dou4-tu3-dou4
```

After speaking a command word, you should see the command word recognition result:

```text
[WARN] [speech_agent_audio]: cmd word:向前走
```

# API Reference

## Topics

| Name | Message Type | Description |
| --- | --- | --- |
| /wkp_text | std_msgs/msg/String | Wake-up word text |
| /audio_smart | audio_msg/msg/SmartAudioData | Command word recognition result |
| /asr_data | audio_msg/msg/SmartAudioData | Noise-reduced ASR audio data |
| /vad_data | audio_msg/msg/SmartAudioData | VAD status events (BOS/MID/EOS/timeout) |
| /doa_data | audio_msg/msg/SmartAudioData | Direction of Arrival (DOA) angle |

## Parameters

| Parameter | Type | Description | Required | Supported Values | Default |
| --- | --- | --- | --- | --- | --- |
| micphone_name | string | Audio capture device name | No | Configure as needed | plughw:0,1 |
| mic_type | string | Microphone type | No | 4mic, 6mic | 4mic |
| wkp_event_pub_topic_name | string | Wake-up event publish topic | No | Configure as needed | /wkp_text |
| doa_pub_topic_name | string | DOA angle publish topic | No | Configure as needed | /doa_data |
| cmd_pub_topic_name | string | Command word result publish topic | No | Configure as needed | /audio_smart |
| asr_pub_topic_name | string | ASR audio data publish topic | No | Configure as needed | /asr_data |
| vad_pub_topic_name | string | VAD data publish topic | No | Configure as needed | /vad_data |
| vad_eos_timeout | int | VAD end-of-speech timeout (ms) | No | Configure as needed | 500 |

## Runtime Dependencies

- ALSA (libasound) — audio capture
- VTN SDK (libvtn.so) — noise reduction and wake-up engine
- VAD SDK (libevad.so) — voice activity detection
- speech_agent_bootstrap — SDK initialization wrapper library
- jsoncpp — command word configuration parsing

# FAQ

1. **Unable to open the audio device?**
   - Verify that the audio board is properly connected.
   - Confirm the `micphone_name` parameter is configured correctly (typically `plughw:0,1`).
   - Use `arecord -l` to list available audio devices.

2. **What is the difference between 4-mic and 6-mic?**
   - **4-mic**: Uses a linear 4-mic audio board to capture 8-channel audio, remapped to 6 channels before entering the VTN engine.
   - **6-mic**: Uses a circular 6-mic audio board to capture 8-channel audio, fed directly into the VTN engine (no channel remapping).

3. **Wake-up word not working?**
   - Confirm that the VTN engine initialized successfully (the log should show "speech engine init success").
   - Verify that the `vtn.ini` configuration file exists.