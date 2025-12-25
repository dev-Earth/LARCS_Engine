#include <fmt/core.h>

#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  CLI::App app{"LARCS Record Tool - Log recording (skeleton)"};

  std::string output_file;
  std::vector<std::string> topics;
  int duration = 0;

  app.add_option("-o,--output", output_file, "Output file for recording")->required();
  app.add_option("-t,--topics", topics, "Topics to record")->required();
  app.add_option("-d,--duration", duration, "Recording duration in seconds (0 = until Ctrl+C)")
      ->default_val(0);

  CLI11_PARSE(app, argc, argv);

  fmt::print("LARCS RECORD\n");
  fmt::print("Output file: {}\n", output_file);
  fmt::print("Topics: ");
  for (const auto& topic : topics) {
    fmt::print("{} ", topic);
  }
  fmt::print("\n");
  fmt::print("Duration: {} seconds\n", duration == 0 ? "unlimited" : std::to_string(duration));

  fmt::print("\nNOTE: Recording functionality not yet implemented.\n");
  fmt::print("This is a skeleton for future implementation.\n");

  return 0;
}
