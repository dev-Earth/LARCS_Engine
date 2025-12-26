#include "larcs/runtime/publisher.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "geometry.pb.h"

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <google/protobuf/util/json_util.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

int main(int argc, char* argv[]) {
  CLI::App app{"LARCS Publisher - Publish protobuf messages to a topic"};

  std::string topic;
  std::string json_message;
  std::string message_type = "Twist";
  bool verbose = false;

  app.add_option("topic", topic, "Topic name to publish to")->required();
  app.add_option("message", json_message, "Message in JSON format")->required();
  app.add_option("-t,--type", message_type,
                 "Message type (Twist, Pose, Vector3)")
      ->default_val("Twist");
  app.add_flag("-v,--verbose", verbose, "Enable verbose logging");

  CLI11_PARSE(app, argc, argv);

  // Configure logging
  if (verbose) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  // Initialize transport
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  if (!transport->initialize()) {
    fmt::print(stderr, "Error: Failed to initialize Zenoh transport\n");
    return 1;
  }

  fmt::print("Publishing to topic: {}\n", topic);
  fmt::print("Message type: {}\n", message_type);
  fmt::print("JSON: {}\n", json_message);

  bool success = false;

  if (message_type == "Twist") {
    larcs::msgs::Twist msg;
    auto status = google::protobuf::util::JsonStringToMessage(json_message, &msg);
    if (!status.ok()) {
      fmt::print(stderr, "Error: Failed to parse JSON: {}\n",
                 std::string(status.message()));
      return 1;
    }

    auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
        transport, topic);
    success = pub->publish(msg);

  } else if (message_type == "Pose") {
    larcs::msgs::Pose msg;
    auto status = google::protobuf::util::JsonStringToMessage(json_message, &msg);
    if (!status.ok()) {
      fmt::print(stderr, "Error: Failed to parse JSON: {}\n",
                 std::string(status.message()));
      return 1;
    }

    auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Pose>>(
        transport, topic);
    success = pub->publish(msg);

  } else if (message_type == "Vector3") {
    larcs::msgs::Vector3 msg;
    auto status = google::protobuf::util::JsonStringToMessage(json_message, &msg);
    if (!status.ok()) {
      fmt::print(stderr, "Error: Failed to parse JSON: {}\n",
                 std::string(status.message()));
      return 1;
    }

    auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Vector3>>(
        transport, topic);
    success = pub->publish(msg);

  } else {
    fmt::print(stderr, "Error: Unsupported message type: {}\n", message_type);
    return 1;
  }

  if (success) {
    fmt::print("Successfully published message\n");
    transport->shutdown();
    return 0;
  } else {
    fmt::print(stderr, "Error: Failed to publish message\n");
    transport->shutdown();
    return 1;
  }
}
