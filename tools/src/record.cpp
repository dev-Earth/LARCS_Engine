#include "larcs/runtime/recorder.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/runtime/subscriber.hpp"
#include "larcs/runtime/time.hpp"

#include <fmt/core.h>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>

static std::atomic<bool> g_shutdown{false};

void signal_handler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    g_shutdown.store(true);
  }
}

std::string generate_filename() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&time_t);
  
  std::ostringstream oss;
  oss << "recording_" 
      << std::put_time(&tm, "%Y%m%d_%H%M%S")
      << ".mcap";
  
  return oss.str();
}

int main(int argc, char** argv) {
  CLI::App app{"LARCS Record Tool - Log MCAP recording"};

  std::string output_file;
  std::vector<std::string> topics;
  int duration = 0;
  bool verbose = false;

  app.add_option("-o,--output", output_file, "Output file for recording");
  app.add_option("-t,--topics", topics, "Topics to record (default: all topics)");
  app.add_option("-d,--duration", duration, "Recording duration in seconds (0 = until Ctrl+C)")
      ->default_val(0);
  app.add_flag("-v,--verbose", verbose, "Enable verbose logging");

  CLI11_PARSE(app, argc, argv);

  // Configure logging
  if (verbose) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  // Generate output filename if not provided
  if (output_file.empty()) {
    output_file = generate_filename();
  }

  fmt::print("LARCS RECORD\n");
  fmt::print("Output file: {}\n", output_file);
  fmt::print("Topics: ");
  if (topics.empty()) {
    fmt::print("all\n");
  } else {
    for (const auto& topic : topics) {
      fmt::print("{} ", topic);
    }
    fmt::print("\n");
  }
  fmt::print("Duration: {} seconds\n", duration == 0 ? "unlimited" : std::to_string(duration));

  // Setup signal handler
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  // Initialize transport
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  if (!transport->initialize()) {
    fmt::print(stderr, "Error: Failed to initialize Zenoh transport\n");
    return 1;
  }

  // Create recorder
  larcs::runtime::Recorder recorder(output_file);
  if (!recorder.open()) {
    fmt::print(stderr, "Error: Failed to open MCAP recorder\n");
    transport->shutdown();
    return 1;
  }

  fmt::print("\nRecording started. Press Ctrl+C to stop.\n");

  // Recording loop
  auto start_time = std::chrono::steady_clock::now();
  while (!g_shutdown.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (duration > 0) {
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::steady_clock::now() - start_time).count();
      
      if (elapsed >= duration) {
        fmt::print("\nRecording duration reached.\n");
        break;
      }
    }
  }

  fmt::print("\nStopping recording...\n");
  recorder.close();
  transport->shutdown();

  fmt::print("Recording saved to: {}\n", output_file);

  return 0;
}
