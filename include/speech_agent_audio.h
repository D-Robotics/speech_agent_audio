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

#ifndef SPEECH_AGENT_AUDIO_INCLUDE_H_
#define SPEECH_AGENT_AUDIO_INCLUDE_H_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <alsa/asoundlib.h>
#include "audio_msg/msg/smart_audio_data.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "utils/alsa_device.h"
#include "vtn_api.h"
#include "vtn_param.h"
#include "general_macros.h"
#include "vad.h"
#include "speech_agent_bootstrap/speech_agent_bootstrap.h"

namespace speech_agent_audio {

using rclcpp::NodeOptions;
class SpeechAgentAudioNode : public rclcpp::Node {
 public:
  // node_name为创建的节点名，options为选项，用法和ROS Node相同
  SpeechAgentAudioNode(const std::string& node_name,
                 const NodeOptions& options = NodeOptions());

  virtual ~SpeechAgentAudioNode();

  int Run();

 public:
  int DeviceInit();
  int SpeechEngineInit();
  int VadInit();
  void CmdInit();
  int DeInit();
  int Start();
  int Stop();

 private:
  void MicphoneGetThread();
  static void ParseAuthInfo(const char *info);
  static void ParseWakeupEvent(const char *info, void *user_data);
  static void ParseWakeupDetailEvent(const char *info, void *user_data);
  static void FeedAudioToVad(const char *audio, int len, void *user_data);
  static void VadCallback(int status, const char* audio, int aLen, void* user_data);
  static int VtnCallbackHandler(const vtn_callback_data_t *data_p, void *user_data);
  std::string TrimSpace(std::string s);
  std::string Utf8ToGbk(std::string utf8_str);
  std::vector<std::string> ParseCmdFile();
  void EvaluateCmdWord(const std::vector<std::string>& cmd_list);
  void GenerateCmdRes(const std::string& cmd_res_file_path, const std::vector<std::string>& cmd_list);
  void AddCmdWordRes(const std::string& cmd_res_file_path);

 private:
  alsa_device_t* micphone_device_ = nullptr;
  bool is_init_ = false;
  int audio_num_ = 0;
  std::string micphone_name_ = "plughw:0,1";
  std::string mic_type_ = "4mic"; 
  int micphone_rate_ = 16000;
  int micphone_chn_ = 8;
  int micphone_buffer_time_ = 0;
  int micphone_nperiods_ = 4;
  int micphone_period_size_ = 1024;
  int vad_eos_timeout_ = 500;
  proc_vtn_api_interact proc_interact_ = nullptr;
  std::atomic<bool> is_vad_flag_ = false;
  std::string cfg_root_path_;
  std::set<std::string> cmd_words_;
  std::string speech_res_path_ = "/lib/speech_agent_audio/";
  std::string vtn_res_path_ = "res/vtn/vtn.ini";
  std::string vad_res_path_ = "/res/vad/evad_5f30adb7.jet";
  std::string cmd_res_path_ = "/res/cmd_word/";
  std::string cmd_res_file_name_ = "cmd_res.bin";
  std::string cmd_file_name_ = "cmd_word.json";
  speech_agent_vtn_ctx_t vtn_ctx_{};
  VADHandle vad_handle_ = nullptr;
  std::thread cmd_init_thread_;

  std::string wkp_event_pub_topic_name_ = "/wkp_text";  // 唤醒事件
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr wkp_event_publisher_;

  std::string doa_pub_topic_name_ = "/doa_data";        // doa数据
  rclcpp::Publisher<audio_msg::msg::SmartAudioData>::SharedPtr doa_publisher_;

  std::string cmd_pub_topic_name_ = "/audio_smart";        // 命令词结果
  rclcpp::Publisher<audio_msg::msg::SmartAudioData>::SharedPtr cmd_publisher_;

  std::string asr_pub_topic_name_ = "/asr_data";        // asr音频数据
  rclcpp::Publisher<audio_msg::msg::SmartAudioData>::SharedPtr asr_data_publisher_;

  std::string vad_pub_topic_name_ = "/vad_data";         // vad数据
  rclcpp::Publisher<audio_msg::msg::SmartAudioData>::SharedPtr vad_data_publisher_;
};

}  // namespace speech_agent_audio

#endif
