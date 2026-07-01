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
from launch.actions import ExecuteProcess, DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():
    pkg_share = get_package_share_directory('speech_agent_audio')
    setup_env = os.path.join(pkg_share, 'setup_env.sh')

    micphone_name = LaunchConfiguration('micphone_name')
    mic_type = LaunchConfiguration('mic_type')
    push_wakeup = LaunchConfiguration('push_wakeup')
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
            'push_wakeup', default_value='0',
            description='push wakeup_name before asr data'),
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

        ExecuteProcess(
            cmd=['bash', '-c',
                 'source ' + setup_env + ' && '
                 'exec ros2 run speech_agent_audio speech_agent_audio '
                 '--ros-args --log-level warn '
                 '-p micphone_name:="$1" '
                 '-p mic_type:="$2" '
                 '-p push_wakeup:="$3" '
                 '-p vad_eos_timeout:="$4" '
                 '-p wkp_event_pub_topic_name:="$5" '
                 '-p doa_pub_topic_name:="$6" '
                 '-p cmd_pub_topic_name:="$7" '
                 '-p asr_pub_topic_name:="$8" '
                 '-p vad_pub_topic_name:="$9"',
                 'bash',
                 micphone_name,
                 mic_type,
                 push_wakeup,
                 vad_eos_timeout,
                 wkp_topic,
                 doa_topic,
                 cmd_topic,
                 asr_audio_topic,
                 vad_topic],
            output='screen',
        ),
    ])