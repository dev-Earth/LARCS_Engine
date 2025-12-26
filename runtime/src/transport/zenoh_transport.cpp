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
  z_owned_config_t config;
  if (z_config_default(&config) != Z_OK) {
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
  z_config_insert_json(z_loan(config), Z_CONFIG_MODE_KEY, "\"peer\"");
  z_config_insert_json(z_loan(config), Z_CONFIG_MULTICAST_SCOUTING_KEY,
                       "\"true\"");

  // Open Zenoh session
  if (z_open(&session_, z_move(config), NULL) != Z_OK) {
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
  z_close(z_move(session_), NULL);
  session_ = z_session_null();

  running_ = false;
  spdlog::info("Zenoh transport shutdown complete");
}

bool ZenohTransport::is_running() const { return running_; }

}  // namespace larcs::runtime
