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
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
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
    push_wakeup = LaunchConfiguration('push_wakeup')
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
            'asr_pub_topic', default_value='/asr_text',
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
        # Include individual launch files
        # ----------------------------------------------------------
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(audio_launch),
            launch_arguments=[
                ('micphone_name', micphone_name),
                ('mic_type', mic_type),
                ('push_wakeup', push_wakeup),
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
    ])