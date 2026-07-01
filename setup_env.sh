#!/bin/bash
# source setup_env.sh

_SDK_ROOT="/opt/tros/humble/lib/speech_agent_audio"
export LD_LIBRARY_PATH="${_SDK_ROOT}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
