#include "larcs/runtime/replayer.hpp"
#include "larcs/runtime/zenoh_transport.hpp"

#include <fmt/core.h>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  CLI::App app{"LARCS Replay Tool - Log MCAP replay"};

  std::string input_file;
  double rate = 1.0;
  bool loop = false;
  bool verbose = false;
  bool info_only = false;

  app.add_option("-i,--input", input_file, "Input file for replay")->required();
  app.add_option("-r,--rate", rate, "Playback rate multiplier")->default_val(1.0);
  app.add_flag("-l,--loop", loop, "Loop playback");
  app.add_flag("-v,--verbose", verbose, "Enable verbose logging");
  app.add_flag("--info", info_only, "Show file info only, don't replay");

  CLI11_PARSE(app, argc, argv);

  // Configure logging
  if (verbose) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  fmt::print("LARCS REPLAY\n");
  fmt::print("Input file: {}\n", input_file);

  // Create replayer
  larcs::runtime::Replayer replayer(input_file);
  if (!replayer.open()) {
    fmt::print(stderr, "Error: Failed to open MCAP file\n");
    return 1;
  }

  // Get file info
  auto info = replayer.get_info();
  
  fmt::print("\nFile Information:\n");
  fmt::print("  Start time: {} ns\n", info.start_time_ns);
  fmt::print("  End time: {} ns\n", info.end_time_ns);
  fmt::print("  Duration: {:.2f} seconds\n", 
            (info.end_time_ns - info.start_time_ns) / 1e9);
  fmt::print("  Message count: {}\n", info.message_count);
  fmt::print("  Topics ({}): ", info.topics.size());
  for (const auto& topic : info.topics) {
    fmt::print("{} ", topic);
  }
  fmt::print("\n");

  if (info_only) {
    replayer.close();
    return 0;
  }

  fmt::print("\nPlayback rate: {}x\n", rate);
  fmt::print("Loop: {}\n", loop ? "yes" : "no");

  // Initialize transport
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  if (!transport->initialize()) {
    fmt::print(stderr, "Error: Failed to initialize Zenoh transport\n");
    replayer.close();
    return 1;
  }

  fmt::print("\nStarting replay. Press Ctrl+C to stop.\n");

  // Start replay
  larcs::runtime::Replayer::ReplayOptions options;
  options.rate = rate;
  options.loop = loop;

  bool success = replayer.replay(transport, options);

  replayer.close();
  transport->shutdown();

  if (success) {
    fmt::print("\nReplay completed successfully.\n");
    return 0;
  } else {
    fmt::print(stderr, "\nReplay failed.\n");
    return 1;
  }
}
