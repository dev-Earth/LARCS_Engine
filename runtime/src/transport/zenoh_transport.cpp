#include "larcs/runtime/zenoh_transport.hpp"

#include "larcs/runtime/logger.hpp"

#include <spdlog/spdlog.h>

namespace larcs::runtime {

ZenohTransport::ZenohTransport() : running_(false) {
  // Initialize session to null/invalid state
  session_ = z_session_null();
}

ZenohTransport::~ZenohTransport() {
  if (running_) {
    shutdown();
  }
}

bool ZenohTransport::initialize(const std::string& config_path) {
  if (running_) {
    spdlog::warn("ZenohTransport already initialized");
    return true;
  }

  // Create Zenoh configuration
  z_owned_config_t config = z_config_default();
  if (!z_check(config)) {
    spdlog::error("Failed to create Zenoh default config");
    return false;
  }

  // If config_path is provided, load from file
  if (!config_path.empty()) {
    spdlog::info("Loading Zenoh config from: {}", config_path);
    // Note: zenoh-c doesn't have direct file loading in all versions
    // For now, we use default config
    spdlog::warn("Config file loading not yet implemented, using defaults");
  }

  // Set peer mode with multicast scouting for automatic discovery
  zp_config_insert(z_loan(config), Z_CONFIG_MODE_KEY, z_string_make("peer"));
  zp_config_insert(z_loan(config), Z_CONFIG_MULTICAST_SCOUTING_KEY,
                   z_string_make("true"));

  // Open Zenoh session
  session_ = z_open(z_move(config));
  if (!z_check(session_)) {
    spdlog::error("Failed to open Zenoh session");
    return false;
  }

  running_ = true;
  spdlog::info("Zenoh transport initialized successfully");
  return true;
}

void ZenohTransport::shutdown() {
  if (!running_) {
    return;
  }

  spdlog::info("Shutting down Zenoh transport");

  // Close the session
  z_close(z_move(session_));
  session_ = z_session_null();

  running_ = false;
  spdlog::info("Zenoh transport shutdown complete");
}

bool ZenohTransport::is_running() const { return running_; }

}  // namespace larcs::runtime
