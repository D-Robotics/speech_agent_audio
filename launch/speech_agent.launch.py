#!/usr/bin/env python3
# Copyright (c) 2024，D-Robotics.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, ExecuteProcess,
                            IncludeLaunchDescription, LogInfo)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():
    # ============================================================
    # speech_agent_audio parameters
    # ============================================================
    micphone_name = LaunchConfiguration('micphone_name')
    mic_type = LaunchConfiguration('mic_type')
    vad_eos_timeout = LaunchConfiguration('vad_eos_timeout')
    wkp_topic = LaunchConfiguration('wkp_topic')
    doa_topic = LaunchConfiguration('doa_topic')
    cmd_topic = LaunchConfiguration('cmd_topic')
    asr_audio_topic = LaunchConfiguration('asr_audio_topic')
    vad_topic = LaunchConfiguration('vad_topic')

    # ============================================================
    # speech_agent_asr parameters
    # ============================================================
    vad_sub_topic = LaunchConfiguration('vad_sub_topic')
    asr_sub_topic = LaunchConfiguration('asr_sub_topic')
    asr_pub_topic = LaunchConfiguration('asr_pub_topic')

    # ============================================================
    # speech_agent_tts parameters
    # ============================================================
    playback_device = LaunchConfiguration('playback_device')
    tts_topic_sub = LaunchConfiguration('tts_topic_sub')

    # ============================================================
    # hobot_llamacpp parameters
    # ============================================================
    feed_type = LaunchConfiguration('feed_type')
    system_prompt = LaunchConfiguration('system_prompt')
    llm_model_name = LaunchConfiguration('llm_model_name')

    # ============================================================
    # Package share directories
    # ============================================================
    audio_pkg_share = get_package_share_directory('speech_agent_audio')
    asr_pkg_share = get_package_share_directory('speech_agent_asr')
    tts_pkg_share = get_package_share_directory('speech_agent_tts')

    audio_launch = os.path.join(audio_pkg_share, 'launch', 'speech_agent_audio.launch.py')
    asr_launch = os.path.join(asr_pkg_share, 'launch', 'speech_agent_asr.launch.py')
    tts_launch = os.path.join(tts_pkg_share, 'launch', 'speech_agent_tts.launch.py')

    return LaunchDescription([
        # ----------------------------------------------------------
        # speech_agent_audio arguments
        # ----------------------------------------------------------
        DeclareLaunchArgument(
            'micphone_name', default_value='plughw:0,1',
            description='audio capture device'),
        DeclareLaunchArgument(
            'mic_type', default_value='4mic',
            description='mic type: [4mic, 6mic]'),
        DeclareLaunchArgument(
            'vad_eos_timeout', default_value='500',
            description='vad eos timeout (ms)'),
        DeclareLaunchArgument(
            'wkp_topic', default_value='/wkp_text',
            description='wakeup event publish topic'),
        DeclareLaunchArgument(
            'doa_topic', default_value='/doa_data',
            description='DOA result publish topic'),
        DeclareLaunchArgument(
            'cmd_topic', default_value='/audio_smart',
            description='command word result publish topic'),
        DeclareLaunchArgument(
            'asr_audio_topic', default_value='/asr_data',
            description='audio data for ASR'),
        DeclareLaunchArgument(
            'vad_topic', default_value='/vad_data',
            description='VAD data publish topic'),

        # ----------------------------------------------------------
        # speech_agent_asr arguments
        # ----------------------------------------------------------
        DeclareLaunchArgument(
            'vad_sub_topic', default_value='/vad_data',
            description='VAD data subscription topic'),
        DeclareLaunchArgument(
            'asr_sub_topic', default_value='/asr_data',
            description='ASR audio data subscription topic'),
        DeclareLaunchArgument(
            'asr_pub_topic', default_value='/prompt_text',
            description='ASR result publication topic'),

        # ----------------------------------------------------------
        # speech_agent_tts arguments
        # ----------------------------------------------------------
        DeclareLaunchArgument(
            'playback_device', default_value='plughw:0,0',
            description='audio playback device'),
        DeclareLaunchArgument(
            'tts_topic_sub', default_value='/tts_text',
            description='tts text subscription topic'),

        # ----------------------------------------------------------
        # hobot_llamacpp arguments
        # ----------------------------------------------------------
        DeclareLaunchArgument(
            'feed_type', default_value='2',
            description='LLM feed type'),
        DeclareLaunchArgument(
            'system_prompt', default_value='config/system_prompt.txt',
            description='LLM system prompt file'),
        DeclareLaunchArgument(
            'llm_model_name', default_value='qwen2.5-0.5b-instruct-q4_0.gguf',
            description='LLM model file name'),

        # ----------------------------------------------------------
        # Usage examples
        # ----------------------------------------------------------
        LogInfo(
            msg=["\n",
                 "========== speech_agent.launch.py usage ==========\n",
                 "usage:\n",
                 "  ros2 launch speech_agent_audio speech_agent.launch.py ",
                 "[<argument_name>:=<value> ...]\n",
                 "\n",
                 "arguments (all optional, pass as <name>:=<value>):\n",
                 "  [speech_agent_audio]\n",
                 "  micphone_name    audio capture device,              default: 'plughw:0,1'\n",
                 "  mic_type         mic type: [4mic, 6mic],            default: '4mic'\n",
                 "  vad_eos_timeout  vad eos timeout (ms),              default: '500'\n",
                 "  wkp_topic        wakeup event publish topic,        default: '/wkp_text'\n",
                 "  doa_topic        DOA result publish topic,          default: '/doa_data'\n",
                 "  cmd_topic        command word result publish topic, default: '/audio_smart'\n",
                 "  asr_audio_topic  audio data for ASR,                default: '/asr_data'\n",
                 "  vad_topic        VAD data publish topic,            default: '/vad_data'\n",
                 "  [speech_agent_asr]\n",
                 "  vad_sub_topic    VAD data subscription topic,       default: '/vad_data'\n",
                 "  asr_sub_topic    ASR audio data subscription topic, default: '/asr_data'\n",
                 "  asr_pub_topic    ASR result publication topic,      default: '/prompt_text'\n",
                 "  [speech_agent_tts]\n",
                 "  playback_device  audio playback device,             default: 'plughw:0,0'\n",
                 "  tts_topic_sub    tts text subscription topic,       default: '/tts_text'\n",
                 "  [hobot_llamacpp]\n",
                 "  feed_type        LLM feed type,                     default: '2'\n",
                 "  system_prompt    system prompt file,                default: 'config/system_prompt.txt'\n",
                 "  llm_model_name   LLM model file name,               default: 'qwen2.5-0.5b-instruct-q4_0.gguf'\n",
                 "\n",
                 "examples:\n",
                 "  # launch with all default values\n",
                 "  ros2 launch speech_agent_audio speech_agent.launch.py\n",
                 "  # launch with 6-mic configuration\n",
                 "  ros2 launch speech_agent_audio speech_agent.launch.py ",
                 "mic_type:=6mic\n",
                 "  # launch with ASR result published to /prompt_text\n",
                 "  ros2 launch speech_agent_audio speech_agent.launch.py ",
                 "asr_pub_topic:=/prompt_text\n",
                 "\n",
                 "current values:\n",
                 "  micphone_name=", micphone_name, "\n",
                 "  mic_type=", mic_type, "\n",
                 "  vad_eos_timeout=", vad_eos_timeout, "\n",
                 "  wkp_topic=", wkp_topic, "\n",
                 "  doa_topic=", doa_topic, "\n",
                 "  cmd_topic=", cmd_topic, "\n",
                 "  asr_audio_topic=", asr_audio_topic, "\n",
                 "  vad_topic=", vad_topic, "\n",
                 "  vad_sub_topic=", vad_sub_topic, "\n",
                 "  asr_sub_topic=", asr_sub_topic, "\n",
                 "  asr_pub_topic=", asr_pub_topic, "\n",
                 "  playback_device=", playback_device, "\n",
                 "  tts_topic_sub=", tts_topic_sub, "\n",
                 "  feed_type=", feed_type, "\n",
                 "  system_prompt=", system_prompt, "\n",
                 "  llm_model_name=", llm_model_name, "\n",
                 "=======================================================\n"]),

        # ----------------------------------------------------------
        # Include individual launch files
        # ----------------------------------------------------------
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(audio_launch),
            launch_arguments=[
                ('micphone_name', micphone_name),
                ('mic_type', mic_type),
                ('vad_eos_timeout', vad_eos_timeout),
                ('wkp_topic', wkp_topic),
                ('doa_topic', doa_topic),
                ('cmd_topic', cmd_topic),
                ('asr_audio_topic', asr_audio_topic),
                ('vad_topic', vad_topic),
            ],
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(asr_launch),
            launch_arguments=[
                ('vad_sub_topic', vad_sub_topic),
                ('asr_sub_topic', asr_sub_topic),
                ('asr_pub_topic', asr_pub_topic),
            ],
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(tts_launch),
            launch_arguments=[
                ('playback_device', playback_device),
                ('topic_sub', tts_topic_sub),
            ],
        ),

        # ----------------------------------------------------------
        # hobot_llamacpp LLM node
        # ----------------------------------------------------------
        ExecuteProcess(
            cmd=['bash', '-c',
                 'exec ros2 run hobot_llamacpp hobot_llamacpp '
                 '--ros-args '
                 '-p feed_type:="$1" '
                 '-p system_prompt:="$2" '
                 '-p llm_model_name:="$3"',
                 'bash',
                 feed_type,
                 system_prompt,
                 llm_model_name],
            output='screen',
        ),
    ])