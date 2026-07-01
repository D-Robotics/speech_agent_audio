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

#include "speech_agent_audio.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <iconv.h>
#include <json/json.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <string>

namespace speech_agent_audio {
std::string getSysTimeMs()
{
    auto tp = std::chrono::system_clock::now();
    auto sec = std::chrono::system_clock::to_time_t(tp);
    auto dur = tp.time_since_epoch();
    // 总毫秒数对1000取余 = 当前毫秒
    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() % 1000;

    std::tm t = *std::localtime(&sec);
    std::ostringstream oss;
    oss << std::put_time(&t,"%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms;
    return oss.str();
}

SpeechAgentAudioNode::SpeechAgentAudioNode(const std::string &node_name,
                               const NodeOptions &options)
    : rclcpp::Node(node_name, options) {
  
  std::string tros_distro
      = std::string(std::getenv("TROS_DISTRO")? std::getenv("TROS_DISTRO") : "");
  cfg_root_path_ = "/opt/tros/" + tros_distro + speech_res_path_;

  this->declare_parameter<int>("push_wakeup", push_wakeup_);
  this->declare_parameter<int>("vad_eos_timeout", vad_eos_timeout_);
  this->declare_parameter<std::string>("micphone_name", micphone_name_);
  this->declare_parameter<std::string>("mic_type", mic_type_);
  this->declare_parameter<std::string>("wkp_event_pub_topic_name", wkp_event_pub_topic_name_);
  this->declare_parameter<std::string>("doa_pub_topic_name", doa_pub_topic_name_);
  this->declare_parameter<std::string>("cmd_pub_topic_name", cmd_pub_topic_name_);
  this->declare_parameter<std::string>("asr_pub_topic_name", asr_pub_topic_name_);
  this->declare_parameter<std::string>("vad_pub_topic_name", vad_pub_topic_name_);

  this->get_parameter<int>("push_wakeup", push_wakeup_);
  this->get_parameter<int>("vad_eos_timeout", vad_eos_timeout_);
  this->get_parameter<std::string>("micphone_name", micphone_name_);
  this->get_parameter<std::string>("mic_type", mic_type_);
  this->get_parameter<std::string>("wkp_event_pub_topic_name", wkp_event_pub_topic_name_);
  this->get_parameter<std::string>("doa_pub_topic_name", doa_pub_topic_name_);
  this->get_parameter<std::string>("cmd_pub_topic_name", cmd_pub_topic_name_);
  this->get_parameter<std::string>("asr_pub_topic_name", asr_pub_topic_name_);
  this->get_parameter<std::string>("vad_pub_topic_name", vad_pub_topic_name_);

  std::stringstream ss;
  ss << "Parameter:"
     << "\n micphone_name: " << micphone_name_
     << "\n mic_type: " << mic_type_
     << "\n wkp_event_pub_topic_name: " << wkp_event_pub_topic_name_
     << "\n doa_pub_topic_name: " << doa_pub_topic_name_
     << "\n cmd_pub_topic_name: " << cmd_pub_topic_name_
     << "\n asr_pub_topic_name: " << asr_pub_topic_name_
     << "\n vad_pub_topic_name: " << vad_pub_topic_name_
     << "\n push_wakeup: " << push_wakeup_
     << "\n vad_eos_timeout: " << vad_eos_timeout_;
  RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "%s", ss.str().c_str());

  bool init_ok = true;

  if (0 != DeviceInit()) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "micphone device init failed");
    init_ok = false;
  }

  if (0 != SpeechEngineInit()) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "speech engine init failed");
    init_ok = false;
  }

  if (0 != VadInit()) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "vad init failed");
    init_ok = false;
  }

  if (init_ok) {
    is_init_ = true;
  }
  cmd_init_thread_ = std::thread(std::bind(&SpeechAgentAudioNode::CmdInit, this));
}

SpeechAgentAudioNode::~SpeechAgentAudioNode() { DeInit(); }

int SpeechAgentAudioNode::DeviceInit() {
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "init to capture audio");
  micphone_device_ = alsa_device_allocate();
  if (!micphone_device_) {
    RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "open mic device fail");
    return -1;
  }
  /* init micphone device*/
  micphone_device_->name = const_cast<char *>(micphone_name_.c_str());
  micphone_device_->format = SND_PCM_FORMAT_S16;
  micphone_device_->direct = SND_PCM_STREAM_CAPTURE;
  micphone_device_->rate = micphone_rate_;
  micphone_device_->channels = micphone_chn_;
  micphone_device_->buffer_time = micphone_buffer_time_;
  micphone_device_->nperiods = micphone_nperiods_;
  micphone_device_->period_size = micphone_period_size_;
  int ret = alsa_device_init(micphone_device_);
  if (ret < 0) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"),
                 "alsa device init fail, ret=%d", ret);
    return -1;
  }
  return 0;
}

int SpeechAgentAudioNode::SpeechEngineInit() {
  // 使用 speech_agent_bootstrap 替代原来的动态库加载和 init
  int ret = speech_agent_vtn_init(&vtn_ctx_,
                                  (void*)VtnCallbackHandler,
                                  this,
                                  cfg_root_path_.c_str(),
                                  mic_type_.c_str());
  if (ret != 0) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "speech_agent_vtn_init failed");
    return -1;
  }
  proc_interact_ = (proc_vtn_api_interact)vtn_ctx_.proc_interact;

  wkp_event_publisher_ = this->create_publisher<std_msgs::msg::String>(wkp_event_pub_topic_name_, 10);
  doa_publisher_ = this->create_publisher<audio_msg::msg::SmartAudioData>(doa_pub_topic_name_, 10);
  cmd_publisher_ = this->create_publisher<audio_msg::msg::SmartAudioData>(cmd_pub_topic_name_, 10);
  asr_data_publisher_ = this->create_publisher<audio_msg::msg::SmartAudioData>(asr_pub_topic_name_, 10);
  vad_data_publisher_ = this->create_publisher<audio_msg::msg::SmartAudioData>(vad_pub_topic_name_, 10);
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "speech engine init success");
  return 0;
}

int SpeechAgentAudioNode::VadInit() {
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "vad init");
  void* vad_handle = nullptr;
  int ret = speech_agent_vad_init(&vad_handle,
                                  (void*)VadCallback,
                                  this,
                                  cfg_root_path_.c_str(),
                                  vad_eos_timeout_);
  if (ret != 0) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "speech_agent_vad_init failed:%d", ret);
    return -1;
  }
  vad_handle_ = (VADHandle)vad_handle;
  return 0;
}

std::string SpeechAgentAudioNode::TrimSpace(std::string s) {
  // 去除行首的空格、Tab、换行和回车等空白符
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch); 
  }));
  // 去除行尾的空格、Tab、换行和回车等空白符
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
  }).base(), s.end());
  return s;
}

std::string SpeechAgentAudioNode::Utf8ToGbk(std::string utf8_str) {
  if (utf8_str.empty()) {
    return "";
  }
  iconv_t cd = iconv_open("GBK", "UTF-8");
  if (cd == (iconv_t) -1 ) {
    return "";
  }

  size_t in_left = utf8_str.size();
  size_t out_left = in_left * 2 + 1; // GBK 是双字节编码，预留足够空间
  std::vector<char> buffer(out_left);
    
  char* in_buf = const_cast<char*>(utf8_str.c_str());
  char* out_buf = buffer.data();

  // iconv 会改变 in_buf 和 out_buf 的指向，所以传入的是指针的地址
  if (iconv(cd, &in_buf, &in_left, &out_buf, &out_left) == (size_t)-1) {
      iconv_close(cd);
      return "";
  }
  iconv_close(cd);
  return std::string(buffer.data());
}

std::vector<std::string> SpeechAgentAudioNode::ParseCmdFile() {
  std::string cmd_word_path = cfg_root_path_ + cmd_res_path_ + cmd_file_name_;
  std::ifstream cmd_word_file(cmd_word_path);
  if (!cmd_word_file.is_open()) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "open cmd word file failed:%s", cmd_word_path.c_str());
    return {};
  }
  Json::Value root;
  Json::Reader reader;
  bool parse_ok = reader.parse(cmd_word_file, root, false);
  if (!parse_ok) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "parse cmd word file failed:%s", cmd_word_path.c_str());
    return {};
  }
  Json::Value cmd_word_arr = root["cmd_word"];
  std::vector<std::string> cmd_list;
  for (int i = 0; i < cmd_word_arr.size(); ++i) {
    Json::Value item = cmd_word_arr[i];
    std::string cmd_text = TrimSpace(item["text"].asString());
    std::string cmd_pinyin = TrimSpace(item["pinyin"].asString());
    RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "cmd word text:%s, pinyin:%s", cmd_text.c_str(), cmd_pinyin.c_str());
    if (!cmd_text.empty() && !cmd_pinyin.empty()) {
      cmd_words_.emplace(cmd_text);
      std::string cmd_word = cmd_text + ";" + cmd_pinyin;
      cmd_list.push_back(cmd_word);
    }
  }
  return cmd_list;
}

void SpeechAgentAudioNode::EvaluateCmdWord(const std::vector<std::string>& cmd_list) {
  for (const auto& cmd_word : cmd_list) {
    // 获取汉字用于评价
    size_t pos = cmd_word.find(";");
    if (pos == std::string::npos) {
      RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "cmd word invalid:%s", cmd_word.c_str());
      continue;
    }
    std::string hanzi = Utf8ToGbk(cmd_word.substr(0, pos));
    if (hanzi.empty()) {
      RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "cmd word does not contain Chinese characters:%s", cmd_word.c_str());
      continue;
    }
    char evaluate_word_buf[128] = {0};
    size_t copy_len = std::min(hanzi.size(), sizeof(evaluate_word_buf) - 1U);
    if (copy_len > 0) {
      memcpy(evaluate_word_buf, hanzi.c_str(), copy_len);
    }
    evaluate_word_buf[copy_len] = '\0';
    DEFINE_INTERACT_INFO(256, 128, 128, 128);
    interact_info.type = VTN_INTERACT_TYPE_EVALUATE_WAKE_WORD;
    interact_info.in.raw_size = 0;
		interact_info.in.str_size = 1U + (unsigned int)snprintf(interact_info.in.str, interact_info.in.str_size - 1U,
																                              "{ \"params\":{ \"word\":\"%s\" } }", evaluate_word_buf);
    if (!proc_interact_) {
      RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "proc interact is null, not evaluate cmd word");
      continue;
    }
    vtn_status_code_e vtn_status = proc_interact_((vtn_handle)vtn_ctx_.vtn_handle, &interact_info);
    if (vtn_status != VTN_STATUS_SUCCESS) {
      RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "evaluate cmd word failed:%d", (int)vtn_status);
      continue;
    }
    if (!(VTN_INTERACT_OUTPUT_TYPE_STR & interact_info.with_out)) {
			RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "shouldn't reach here: no interact result string for:%s", hanzi.c_str());
      continue;
		}
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "evaluate cmd word:%s => %s", cmd_word.substr(0, pos).c_str(), interact_info.out.str);
  }
}

void SpeechAgentAudioNode::GenerateCmdRes(const std::string& cmd_res_file_path, const std::vector<std::string>& cmd_list) {
  // 组合命令词，并用逗号分隔
  std::string cmd_word;
  for (std::size_t i = 0; i < cmd_list.size(); ++i) {
    cmd_word += cmd_list[i];
    if (i != cmd_list.size() - 1) {
      cmd_word += ",";
    }
  }
  RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "generate cmd res for [%s]", cmd_word.c_str());
  // 生成命令词资源
  DEFINE_INTERACT_INFO(4096, 512, 512, 16384);
  interact_info.type = VTN_INTERACT_TYPE_GENERATE_WAKE_WORD_RES;
	interact_info.in.raw_size = 0;
	interact_info.in.str_size = 1U + (unsigned int)snprintf(interact_info.in.str, interact_info.in.str_size - 1U,
																                            "{ \"params\":{ \"word\":\"%s\" } }", cmd_word.c_str());
  if (!proc_interact_) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "proc interact is null, not generate cmd res");
    return;
  }
  vtn_status_code_e vtn_status = proc_interact_((vtn_handle)vtn_ctx_.vtn_handle, &interact_info);
  if (vtn_status != VTN_STATUS_SUCCESS) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "generate cmd res failed:%d", (int)vtn_status);
    return;
  }
  if (!(VTN_INTERACT_OUTPUT_TYPE_RAW & interact_info.with_out)) {
		RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "shouldn't reach here: no interact result string for:%s", cmd_word.c_str());
    return;
	}
  RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "generate cmd res:%s => %zu bytes", cmd_word.c_str(), interact_info.out.raw_size);
  std::ofstream out_file(cmd_res_file_path, std::ios::binary | std::ios::out);
  if (!out_file.is_open()) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "create cmd res failed:%s", cmd_res_file_path.c_str());
    return;
  }
  out_file.write(reinterpret_cast<char*>(interact_info.out.raw), interact_info.out.raw_size);
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "save generated word res to:%s", cmd_res_file_path.c_str());
}

void SpeechAgentAudioNode::AddCmdWordRes(const std::string& cmd_res_file_path) {
  DEFINE_INTERACT_INFO(512, 16384, 512, 512);
  interact_info.type = VTN_INTERACT_TYPE_ADD_WAKE_WORD_RES;
  std::ifstream in_file(cmd_res_file_path, std::ios::binary | std::ios::ate);
  if (!in_file.is_open()) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "open cmd res failed:%s", cmd_res_file_path.c_str());
    return;
  }
  std::streamsize file_size = in_file.tellg();
  if (file_size <= 0) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "cmd res is null:%s", cmd_res_file_path.c_str());
    in_file.close();
    return;
  }
  in_file.seekg(0, std::ios::beg);
  in_file.read(reinterpret_cast<char*>(interact_info.in.raw), file_size);
  if (in_file.gcount() != file_size) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "cmd res size:%zu, but read size:%zd", file_size, in_file.gcount());
    return;
  }
  interact_info.in.raw_size = file_size;
  interact_info.in.str_size = 0;
  if (!proc_interact_) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "proc interact is null, not add cmd res");
    return;
  }
  vtn_status_code_e vtn_status = proc_interact_((vtn_handle)vtn_ctx_.vtn_handle, &interact_info);
  if (vtn_status != VTN_STATUS_SUCCESS) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "add cmd res failed:%d", (int)vtn_status);
    return;
  }
  if (!(VTN_INTERACT_OUTPUT_TYPE_STR & interact_info.with_out)) {
		RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "shouldn't reach here: no interact result string for:%s", cmd_res_file_path.c_str());
    return;
	}
  RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "succeed add_word_res:%s", interact_info.out.str);
}

void SpeechAgentAudioNode::CmdInit() {
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "cmd init");
  // 解析命令词文件
  std::vector<std::string> cmd_list = ParseCmdFile();
  // 评估命令词
  EvaluateCmdWord(cmd_list);
  // 生成命令词资源文件
  std::string cmd_res_file_path = cfg_root_path_ + cmd_res_path_ + cmd_res_file_name_;
  GenerateCmdRes(cmd_res_file_path, cmd_list);
  // 添加命令词资源文件到引擎
  AddCmdWordRes(cmd_res_file_path);
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "cmd init finish");
}

int SpeechAgentAudioNode::DeInit() {
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "deinit");
  if (!is_init_) return 0;
  if (micphone_device_) {
    alsa_device_deinit(micphone_device_);
    alsa_device_free(micphone_device_);
    micphone_device_ = nullptr;
  }
  speech_agent_vtn_deinit(&vtn_ctx_);
  speech_agent_vad_deinit(vad_handle_);
  vad_handle_ = nullptr;
  if (cmd_init_thread_.joinable()) {
    cmd_init_thread_.join();
  }
  is_init_ = false;
  return 0;
}

int SpeechAgentAudioNode::Run() {
  if (!is_init_) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "SpeechAgentAudioNode not init.");
    return -1;
  }

  rclcpp::executors::SingleThreadedExecutor exec;
  auto capture_task = std::make_shared<std::thread>(
      std::bind(&SpeechAgentAudioNode::MicphoneGetThread, this));
  exec.spin();
  if (capture_task && capture_task->joinable()) {
    capture_task->join();
  }
  exec.spin();
  return 0;
}

void SpeechAgentAudioNode::MicphoneGetThread() {
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "start to capture audio");
  if (!micphone_device_) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "micphone device is null");
    return;
  }
  if (!proc_interact_) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "proc interact is null");
    return;
  }
  DEFINE_STR_INTERACT_INFO(1024, 1024);
  RESET_STR_INTERACT_INFO();
  interact_info.type = VTN_INTERACT_TYPE_FEED_AUDIO;
  int ret = -1;
  snd_pcm_sframes_t frames;
  frames = micphone_device_->period_size;
  int buffer_size = snd_pcm_frames_to_bytes(micphone_device_->handle, frames);

  // 8ch -> 6ch (only for 4mic)
  bool is_4mic = (mic_type_ == "4mic");
  int vtn_input_size = is_4mic ? (buffer_size * 6 / 8) : buffer_size;
  char *buffer = new char[buffer_size];
  char *buffer_remapped = nullptr;
  if (is_4mic) {
    buffer_remapped = new char[vtn_input_size];
  }
  RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), ">>> mic type:[%s]", mic_type_.c_str());
  while (rclcpp::ok()) {
    ret = alsa_device_read(micphone_device_, buffer, frames);
    RCLCPP_DEBUG(rclcpp::get_logger("speech_agent_audio"), "alsa_device_read:%d", ret);
    if (ret <= 0) {
      RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "alsa device read failed:%d", ret);
      continue;
    }
    RCLCPP_DEBUG(rclcpp::get_logger("speech_agent_audio"), "capture audio buffer_size:%d",
                 buffer_size);
    if (is_4mic) {
      // 8通道拆成6通道，通道0,1,2,3是mic通道，通道6,7是回采通道
      int16_t *buffer_int16 = reinterpret_cast<int16_t*>(buffer);
      int16_t *buffer_remapped_int16 = reinterpret_cast<int16_t*>(buffer_remapped);
      for (int i = 0; i < buffer_size / 16; ++i) {
        buffer_remapped_int16[6 * i + 0] = buffer_int16[8 * i + 0];
        buffer_remapped_int16[6 * i + 1] = buffer_int16[8 * i + 1];
        buffer_remapped_int16[6 * i + 2] = buffer_int16[8 * i + 2];
        buffer_remapped_int16[6 * i + 3] = buffer_int16[8 * i + 3];
        buffer_remapped_int16[6 * i + 4] = buffer_int16[8 * i + 6];
        buffer_remapped_int16[6 * i + 5] = buffer_int16[8 * i + 7];
      }
      interact_info.in.raw = (void *)(buffer_remapped_int16);
      interact_info.in.raw_size = vtn_input_size;
    } else {
      // 6mic: 直接将8通道音频送给算法
      interact_info.in.raw = (void *)buffer;
      interact_info.in.raw_size = buffer_size;
    }
    vtn_status_code_e vtn_status = proc_interact_((vtn_handle)vtn_ctx_.vtn_handle, &interact_info);
    if (vtn_status != VTN_STATUS_SUCCESS) {
      RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "proc_interact failed:%d", (int)vtn_status);
      break;
    }
  }
  RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "stop capture audio");
  delete[] buffer;
  if (buffer_remapped) {
    delete[] buffer_remapped;
  }
}

void SpeechAgentAudioNode::ParseAuthInfo(const char *info) {
  // "{ \"msg_type\":\"auth\", \"params\":{ \"auth_type\":\"net\", \"result\":\"0\" } }"
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(info, root)) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseAuthInfo parse auth info failed:%s", info);
    return;
  }
  if (!root.isMember("params") || !root["params"].isObject()) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseAuthInfo parse params failed");
    return;
  }
  Json::Value& params = root["params"];
  if (!params.isMember("result") || !params["result"].isString()) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseAuthInfo parse result failed");
    return;
  }
  std::string result = params["result"].asString();
  RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "auth result:%s", result.c_str());
}

void SpeechAgentAudioNode::ParseWakeupEvent(const char *info, void *user_data) {
  SpeechAgentAudioNode *self = static_cast<SpeechAgentAudioNode*>(user_data);
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(info, root)) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseWakeupEvent failed on parse wakeup info:%s", info);
    return;
  }
  if (!root.isMember("params") || !root["params"].isObject()) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseWakeupEvent parse params failed");
    return;
  }
  Json::Value& params = root["params"];
  if (!params.isMember("keyword") || !params["keyword"].isString()
        || !params.isMember("keyword_type") || !params["keyword_type"].isString()) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseWakeupEvent parse keyword and keyword_type failed");
    return;
  }
  std::string keyword = params["keyword"].asString();
  std::string keyword_type = params["keyword_type"].asString();
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "WakeupEvent: %s", info);
  if (keyword_type == VTN_KEYWORD_TYPE_STR_MAIN) {
    if (self->cmd_words_.find(keyword) != self->cmd_words_.end()) {
      // 发送命令词事件
      audio_msg::msg::SmartAudioData::UniquePtr cmd_msg(new audio_msg::msg::SmartAudioData());
      cmd_msg->frame_type.value = cmd_msg->frame_type.SMART_AUDIO_TYPE_CMD_WORD;
      cmd_msg->cmd_word = keyword;
      self->cmd_publisher_->publish(std::move(cmd_msg));
      RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "cmd word:%s", keyword.c_str());
    } else {
      // 重置VAD状态
      self->is_vad_flag_ = true;
      VADReset(self->vad_handle_);
      // 发送主唤醒事件
      auto wkp_msg = std::make_unique<std_msgs::msg::String>();
      wkp_msg->data = keyword;
      RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "wakeup word:%s, %s", keyword.c_str(), getSysTimeMs().c_str());
      self->wkp_event_publisher_->publish(std::move(wkp_msg));
    }
  } else if (keyword_type == VTN_KEYWORD_TYPE_STR_CMD) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "cmd keyword word: %s", keyword_type.c_str());
  } else {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "unknown keyword type: %s", keyword_type.c_str());
  }
}

void SpeechAgentAudioNode::ParseWakeupDetailEvent(const char *info, void *user_data) {
  SpeechAgentAudioNode *self = static_cast<SpeechAgentAudioNode*>(user_data);
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(info, root)) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseWakeupDetailEvent failed on parse wakeup info:%s", info);
    return;
  }
  if (!root.isMember("params") || !root["params"].isObject()) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseWakeupDetailEvent parse params failed");
    return;
  }
  Json::Value& params = root["params"];
  if (!params.isMember("keyword") || !params["keyword"].isString()
        || !params.isMember("keyword_type") || !params["keyword_type"].isString()) {
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "ParseWakeupDetailEvent parse keyword and keyword_type failed");
    return;
  }
  std::string keyword = params["keyword"].asString();
  std::string keyword_type = params["keyword_type"].asString();
  RCLCPP_INFO(rclcpp::get_logger("speech_agent_audio"), "WakeupDetailEvent: %s", info);
  if (keyword_type == VTN_KEYWORD_TYPE_STR_MAIN) {
    if (params.isMember("angle")) {
      // 主唤醒词唤醒或者触发命令词，发送doa数据
      float_t angle = params["angle"].asFloat();
      audio_msg::msg::SmartAudioData::UniquePtr doa_msg(new audio_msg::msg::SmartAudioData());
      doa_msg->frame_type.value = doa_msg->frame_type.SMART_AUDIO_TYPE_DOA;
      doa_msg->doa_theta = angle;
      self->doa_publisher_->publish(std::move(doa_msg));
      RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "angle:%f", angle);
    }
  }
}

void SpeechAgentAudioNode::FeedAudioToVad(const char *audio, int len, void *user_data) {
  SpeechAgentAudioNode *self = static_cast<SpeechAgentAudioNode*>(user_data);
  if (!self->is_vad_flag_) {
    return;
  }
  int ret = VADWriteAudio(self->vad_handle_, audio, len);
  if (ret) {
    RCLCPP_ERROR(rclcpp::get_logger("speech_agent_audio"), "feed audio to vad failed:%d", ret);
  }
}

void SpeechAgentAudioNode::VadCallback(int status, const char* audio, int aLen, void* user_data) {
  SpeechAgentAudioNode *self = static_cast<SpeechAgentAudioNode*>(user_data);
  audio_msg::msg::SmartAudioData::UniquePtr vad_msg(new audio_msg::msg::SmartAudioData());
  vad_msg->frame_type.value = vad_msg->frame_type.SMART_AUDIO_TYPE_EVENT;
  if (status == VAD_STATUS_FIND_BOS) {
    vad_msg->event_type.value = vad_msg->event_type.EVENT_VADBEGIN;
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "vad begin");
  } else if (status == VAD_STATUS_CONTINUOUS) {
    vad_msg->event_type.value = vad_msg->event_type.EVENT_VADMID;
  } else if (status == VAD_STATUS_FIND_EOS) {
    vad_msg->event_type.value = vad_msg->event_type.EVENT_VADEND;
    self->is_vad_flag_ = false;
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "vad end");
  } else if (status == VAD_STATUS_BOS_TIMEOUT) {
    vad_msg->event_type.value = vad_msg->event_type.EVENT_WAITASRTIMEOUT;
    self->is_vad_flag_ = false;
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "wait asr timeout");
  } else if (status == VAD_STATUS_SPEECH_FINISH) {
    self->is_vad_flag_ = false;
    RCLCPP_WARN(rclcpp::get_logger("speech_agent_audio"), "vad status speech finish");
  }
  self->vad_data_publisher_->publish(std::move(vad_msg));
}

int SpeechAgentAudioNode::VtnCallbackHandler(const vtn_callback_data_t *data_p, void *user_data) {
  SpeechAgentAudioNode *self = static_cast<SpeechAgentAudioNode*>(user_data);
  switch (data_p->type) {
    case VTN_CALLBACK_TYPE_AUTHORIZATION: {
      ParseAuthInfo((const char *)data_p->data);  // 鉴权结果
      break;
    }
    case VTN_CALLBACK_TYPE_WAKEUP: {
      // 唤醒事件,不带角度定位信息,触发的比WAKEUP_DETAIL要稍早,通常用于显示UI或播放提示音;一次唤醒会触发WAKEUP/WAKEUP_DETAIL各一次
      ParseWakeupEvent((const char *)data_p->data, user_data);
      break;
    }
    case VTN_CALLBACK_TYPE_WAKEUP_DETAIL: {
      // 唤醒事件详情,带有角度定位详情信息(以及唤醒词段音频),比WAKEUP触发稍晚,通常用于定位等业务逻辑处理;
      ParseWakeupDetailEvent((const char *)data_p->data, user_data);
      break;
    }
    case VTN_CALLBACK_TYPE_AUDIO_REC: {
      // <-- 降噪后的识别音频(全向), 识别音频会在唤醒后主动输出. 集成开发者可以用此通道音频做文本识别.
      std::vector<uint8_t> asr_data(data_p->data_size);
      std::memcpy(asr_data.data(), data_p->data, data_p->data_size);
      audio_msg::msg::SmartAudioData::UniquePtr asr_msg(new audio_msg::msg::SmartAudioData());
      asr_msg->frame_type.value = asr_msg->frame_type.SMART_AUDIO_TYPE_ASR_DATA;
      asr_msg->data = asr_data;
      self->asr_data_publisher_->publish(std::move(asr_msg));
      break;
    }
    case VTN_CALLBACK_TYPE_AUDIO_VAD: {
      // <-- VAD 音频, 强抑制, 常用于做VAD端点检测
      FeedAudioToVad(static_cast<const char*>(data_p->data), data_p->data_size, user_data);
      break;
    }
    default:
      // 其他类型音频/事件. 没有需求就忽略.
      break;
	}
	return 0;
}

}  // namespace speech_agent_audio
