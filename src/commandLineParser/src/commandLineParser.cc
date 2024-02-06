#include "commandLineParser.hh"

#include <stdlib.h>

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

cxxopts::ParseResult parseFlexer(int argc, char *argv[]) {
  try {
    cxxopts::Options options(argv[0], "");
    options.positional_help("[optional args]").show_positional_help();

    std::string file = "";

    // clang-format off
options.add_options()
  ("dir", "Directory to search for files", cxxopts::value<std::string>())
  ("file", "File to search for flexer instances", cxxopts::value<std::string>())
  ("ss", "List suffixes to search for", cxxopts::value<std::vector<std::string>>())
  ("silent", "Silent mode")
  ("help", "Show options");
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }

    if ((!result.count("dir") && !result.count("file")) ||
        (result.count("dir") && !result.count("ss"))) {
      std::cout << "Usage:\n";
      exit(0);
    }

    return result;

  } catch (const cxxopts::OptionException &e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }
}

