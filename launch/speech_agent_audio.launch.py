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
from launch.actions import ExecuteProcess, DeclareLaunchArgument, LogInfo
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():
    pkg_share = get_package_share_directory('speech_agent_audio')
    setup_env = os.path.join(pkg_share, 'setup_env.sh')

    micphone_name = LaunchConfiguration('micphone_name')
    mic_type = LaunchConfiguration('mic_type')
    vad_eos_timeout = LaunchConfiguration('vad_eos_timeout')
    wkp_topic = LaunchConfiguration('wkp_topic')
    doa_topic = LaunchConfiguration('doa_topic')
    cmd_topic = LaunchConfiguration('cmd_topic')
    asr_audio_topic = LaunchConfiguration('asr_audio_topic')
    vad_topic = LaunchConfiguration('vad_topic')

    return LaunchDescription([
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

        LogInfo(
            msg=["\n",
                 "========== speech_agent_audio.launch.py usage ==========\n",
                 "usage:\n",
                 "  ros2 launch speech_agent_audio speech_agent_audio.launch.py ",
                 "[<argument_name>:=<value> ...]\n",
                 "\n",
                 "arguments (all optional, pass as <name>:=<value>):\n",
                 "  micphone_name    audio capture device,              default: 'plughw:0,1'\n",
                 "  mic_type         mic type: [4mic, 6mic],            default: '4mic'\n",
                 "  vad_eos_timeout  vad eos timeout (ms),              default: '500'\n",
                 "  wkp_topic        wakeup event publish topic,        default: '/wkp_text'\n",
                 "  doa_topic        DOA result publish topic,          default: '/doa_data'\n",
                 "  cmd_topic        command word result publish topic, default: '/audio_smart'\n",
                 "  asr_audio_topic  audio data for ASR,                default: '/asr_data'\n",
                 "  vad_topic        VAD data publish topic,            default: '/vad_data'\n",
                 "\n",
                 "examples:\n",
                 "  # launch with all default values\n",
                 "  ros2 launch speech_agent_audio speech_agent_audio.launch.py\n",
                 "  # launch with 6-mic configuration\n",
                 "  ros2 launch speech_agent_audio speech_agent_audio.launch.py ",
                 "mic_type:=6mic\n",
                 "  # launch with capture device\n",
                 "  ros2 launch speech_agent_audio speech_agent_audio.launch.py ",
                 "micphone_name:=plughw:0,1\n",
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
                 "=======================================================\n"]),

        ExecuteProcess(
            cmd=['bash', '-c',
                 'source ' + setup_env + ' && '
                 'exec ros2 run speech_agent_audio speech_agent_audio '
                 '--ros-args --log-level warn '
                 '-p micphone_name:="$1" '
                 '-p mic_type:="$2" '
                 '-p vad_eos_timeout:="$3" '
                 '-p wkp_event_pub_topic_name:="$4" '
                 '-p doa_pub_topic_name:="$5" '
                 '-p cmd_pub_topic_name:="$6" '
                 '-p asr_pub_topic_name:="$7" '
                 '-p vad_pub_topic_name:="$8"',
                 'bash',
                 micphone_name,
                 mic_type,
                 vad_eos_timeout,
                 wkp_topic,
                 doa_topic,
                 cmd_topic,
                 asr_audio_topic,
                 vad_topic],
            output='screen',
        ),
    ])