// Copyright (c) 2024，D-Robotics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>
#include <string>

#include "speech_agent_audio.h"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"),
              "This is speech agent audio example!");

  std::string node_name = "speech_agent_audio";
  speech_agent_audio::SpeechAgentAudioNode speech_agent_audio(node_name);
  if (speech_agent_audio.Run() != 0) {
      RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"),
                   "Run SpeechAgentAudioNode failed!");
    } else {
      RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"),
                  "Run SpeechAgentAudioNode done!");
    }

  rclcpp::shutdown();
  return 0;
}
