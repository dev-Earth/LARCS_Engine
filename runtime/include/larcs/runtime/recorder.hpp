#pragma once
#include <mcap/writer.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace larcs::runtime {

class Recorder {
public:
  explicit Recorder(const std::string& filepath);
  ~Recorder();
  
  bool open();
  void close();
  bool is_open() const;
  
  // トピックを記録対象に追加
  template<typename MessageT>
  bool add_topic(const std::string& topic);
  
  // メッセージを記録
  template<typename MessageT>
  bool record(const std::string& topic, const MessageT& msg, uint64_t timestamp_ns);
  
private:
  std::string filepath_;
  std::unique_ptr<mcap::McapWriter> writer_;
  std::unordered_map<std::string, mcap::ChannelId> channels_;
  std::mutex mutex_;
  bool open_ = false;
};

} // namespace larcs::runtime
