#include <fmt/core.h>

#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  CLI::App app{"LARCS Replay Tool - Log replay (skeleton)"};

  std::string input_file;
  double rate = 1.0;
  bool loop = false;

  app.add_option("-i,--input", input_file, "Input file for replay")->required();
  app.add_option("-r,--rate", rate, "Playback rate multiplier")->default_val(1.0);
  app.add_flag("-l,--loop", loop, "Loop playback");

  CLI11_PARSE(app, argc, argv);

  fmt::print("LARCS REPLAY\n");
  fmt::print("Input file: {}\n", input_file);
  fmt::print("Playback rate: {}x\n", rate);
  fmt::print("Loop: {}\n", loop ? "yes" : "no");

  fmt::print("\nNOTE: Replay functionality not yet implemented.\n");
  fmt::print("This is a skeleton for future implementation.\n");

  return 0;
}
