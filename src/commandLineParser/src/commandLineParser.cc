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
("help", "Show options");
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }
    if (false) {
      std::cout << "Usage:\n";
      exit(0);
    }

    return result;

  } catch (const cxxopts::OptionException &e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }
}
