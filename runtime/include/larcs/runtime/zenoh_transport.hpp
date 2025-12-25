#pragma once
#include "larcs/runtime/transport.hpp"

#include <zenoh.h>

#include <memory>
#include <string>

namespace larcs::runtime {

class ZenohTransport : public Transport {
 public:
  ZenohTransport();
  ~ZenohTransport() override;

  bool initialize(const std::string& config_path = "") override;
  void shutdown() override;
  bool is_running() const override;

  z_session_t& session() { return session_; }

 private:
  z_session_t session_;
  bool running_ = false;
};

}  // namespace larcs::runtime
