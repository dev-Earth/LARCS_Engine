#include "larcs/runtime/subscriber.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/msgs/geometry.pb.h"

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <google/protobuf/util/json_util.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <csignal>
#include <memory>
#include <string>
#include <thread>

std::atomic<bool> running{true};

void signal_handler(int signal) {
  (void)signal;
  running = false;
}

int main(int argc, char* argv[]) {
  CLI::App app{"LARCS Subscriber - Subscribe to messages from a topic"};

  std::string topic;
  std::string message_type = "Twist";
  int count = -1;
  bool verbose = false;

  app.add_option("topic", topic, "Topic name to subscribe to")->required();
  app.add_option("-t,--type", message_type,
                 "Message type (Twist, Pose, Vector3)")
      ->default_val("Twist");
  app.add_option("-c,--count", count,
                 "Number of messages to receive before exiting (-1 for "
                 "unlimited)")
      ->default_val(-1);
  app.add_flag("-v,--verbose", verbose, "Enable verbose logging");

  CLI11_PARSE(app, argc, argv);

  // Configure logging
  if (verbose) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  // Setup signal handler for clean shutdown
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  // Initialize transport
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  if (!transport->initialize()) {
    fmt::print(stderr, "Error: Failed to initialize Zenoh transport\n");
    return 1;
  }

  fmt::print("Subscribing to topic: {}\n", topic);
  fmt::print("Message type: {}\n", message_type);
  if (count > 0) {
    fmt::print("Will exit after {} messages\n", count);
  } else {
    fmt::print("Listening... (Press Ctrl+C to exit)\n");
  }

  std::atomic<int> received_count{0};

  if (message_type == "Twist") {
    auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
        transport, topic, [&](const larcs::msgs::Twist& msg) {
          std::string json_output;
          google::protobuf::util::JsonPrintOptions options;
          options.add_whitespace = false;
          auto status =
              google::protobuf::util::MessageToJsonString(msg, &json_output, options);
          if (status.ok()) {
            fmt::print("{}\n", json_output);
          } else {
            fmt::print(stderr, "Error converting message to JSON\n");
          }

          received_count++;
          if (count > 0 && received_count >= count) {
            running = false;
          }
        });

    while (running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

  } else if (message_type == "Pose") {
    auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Pose>>(
        transport, topic, [&](const larcs::msgs::Pose& msg) {
          std::string json_output;
          google::protobuf::util::JsonPrintOptions options;
          options.add_whitespace = false;
          auto status =
              google::protobuf::util::MessageToJsonString(msg, &json_output, options);
          if (status.ok()) {
            fmt::print("{}\n", json_output);
          } else {
            fmt::print(stderr, "Error converting message to JSON\n");
          }

          received_count++;
          if (count > 0 && received_count >= count) {
            running = false;
          }
        });

    while (running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

  } else if (message_type == "Vector3") {
    auto sub =
        std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Vector3>>(
            transport, topic, [&](const larcs::msgs::Vector3& msg) {
              std::string json_output;
              google::protobuf::util::JsonPrintOptions options;
              options.add_whitespace = false;
              auto status = google::protobuf::util::MessageToJsonString(
                  msg, &json_output, options);
              if (status.ok()) {
                fmt::print("{}\n", json_output);
              } else {
                fmt::print(stderr, "Error converting message to JSON\n");
              }

              received_count++;
              if (count > 0 && received_count >= count) {
                running = false;
              }
            });

    while (running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

  } else {
    fmt::print(stderr, "Error: Unsupported message type: {}\n", message_type);
    return 1;
  }

  fmt::print("\nReceived {} messages. Shutting down...\n",
             received_count.load());
  transport->shutdown();
  return 0;
}
