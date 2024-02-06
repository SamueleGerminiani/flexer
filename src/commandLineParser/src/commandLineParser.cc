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
  ("project-root", "Directory containing the sources", cxxopts::value<std::string>())
  ("include", "Comma separated list (without spaces) of extensions to search for files (example .cc .hh)", cxxopts::value<std::vector<std::string>>())
  ("server-ip", "IP of the server hosting the flexer service", cxxopts::value<std::string>())
  ("port", "Port of the server hosting the flexer service", cxxopts::value<size_t>())
  ("client", "To specify that flexer is running in client mode")
  ("server", "To specify that flexer is running in server mode")
  ("help", "Show options");
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }

    if (!result.count("project-root") ||
        (!result.count("server") && !result.count("client")) ||
        !result.count("port") || !result.count("server-ip") ||
        !result.count("include")) {
      std::cout << "Usage: flexer --project-root <path> --server-ip <ip> "
                   "--port <port> --include <extensions> --server|--client"
                << std::endl;
      exit(0);
    }

    return result;

  } catch (const cxxopts::OptionException &e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }
}

