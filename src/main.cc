#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "commandLineParser.hh"
#include "flexerIcon.hh"
#include "globals.hh"
#include "message.hh"
#include "text.hh"

/// @brief handle all the command line arguments
static void parseCommandLineArguments(int argc, char* args[]);

/// @brief handles all the unhandled exceptions
void exceptionHandler();

/// @brief handles all the unhandled errors
void handleErrors();

using namespace flexer;

namespace fs = std::filesystem;

std::vector<std::string> findFilesWithExtensions(
    const std::string& directoryPath,
    const std::vector<std::string>& extensions) {
  std::vector<std::string> matchingFilePaths;

  try {
    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
      if (fs::is_regular_file(entry.path())) {
        std::string fileSuffix = entry.path().extension().string();
        // Convert to lowercase for case-insensitive matching
        std::transform(fileSuffix.begin(), fileSuffix.end(), fileSuffix.begin(),
                       ::tolower);

        for (const auto& extension : extensions) {
          if (fileSuffix == extension) {
            matchingFilePaths.push_back(entry.path().string());
            break;  // Move to the next file after a match is found
          }
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return matchingFilePaths;
}

std::vector<std::string> findFiles() {
  std::vector<std::string> inFiles;
  if (!clc::include.empty()) {
    inFiles = findFilesWithExtensions(clc::projectRoot, clc::include);
    messageErrorIf(inFiles.empty(),
                   "No files found in the directory: " + clc::projectRoot);
  } else {
    messageError("No input files or directories found");
  }

  std::cout << "Elaborating the following files: "
            << "\n";
  for (const auto& file : inFiles) {
    std::cout << "\t" << file << "\n";
  }
  return inFiles;
}

int main(int arg, char* argv[]) {
#ifndef DEBUG
  handleErrors();
#endif

  // enforce deterministic rand
  srand(1);

  // print welcome message
  std::cout << getIcon() << "\n";

  parseCommandLineArguments(arg, argv);

  if (clc::client) {
    messageInfo("Client mode");
  } else if (clc::server) {
    messageInfo("Server mode");
  }

  //  // find all the files with the given extensions------------
  //  std::vector<std::string> inFiles = findFiles();
  //
  //  auto instances = extractFlexerInstances(inFiles);
  //  messageErrorIf(instances.empty(), "No flexer instances found");
  //
  //  // debug
  //  // for (const auto& [id, text, startLine, endLine, fileName] : instances)
  //  {
  //  //   std::cout << "ID: " << id << "\n";
  //  //   std::cout << "Text: \n" << text << "\n";
  //  //   std::cout << "Start line: " << startLine << "\n";
  //  //   std::cout << "End line: " << endLine << "\n";
  //  //   std::cout << "File: " << fileName << "\n";
  //  // }
  //
  //  std::unordered_map<std::string, std::vector<FlexerInstance>>
  //  toBeSubstituted =
  //      organizeInstances(instances);
  //
  //  std::unordered_map<std::string, std::vector<FlexerInstance>> subtitutions
  //  =
  //      toBeSubstituted;
  //
  //  size_t i = 0;
  //  for (auto& [file, instances] : subtitutions) {
  //    for (auto& instance : instances) {
  //      instance.text = "substitution_" + std::to_string(i++) + "\n";
  //    }
  //  }
  //
  //  std::unordered_map<std::string, std::vector<FlexerInstance>>
  //      fileToSubInstances = generateSubInstances(toBeSubstituted,
  //      subtitutions);
  //
  //  std::cout <<
  //  "=============================================================="
  //            << "\n";
  //  // print file to sub instances
  //  for (const auto& [fileName, subInstances] : fileToSubInstances) {
  //    std::cout << "File: " << fileName << "\n";
  //    for (const auto& instance : subInstances) {
  //      std::cout << "ID: " << instance.id << "\n";
  //      std::cout << "Text: " << instance.text << "\n";
  //    }
  //  }
  //  std::cout << "==========================================================="
  //            << "\n";
  //
  //  std::cout <<
  //  "---------------------------------------------------------------"
  //               "-------"
  //            << "\n";
  //  for (const auto& [fileName, subInstances] : fileToSubInstances) {
  //    std::cout << "File: " << fileName << "\n";
  //    std::cout << subtituteFlexerInstances(fileName, subInstances) << "\n";
  //  }

  return 0;
}

void exceptionHandler() {
  hlog::dumpErrorToFile("Exception received", errno, -1, true);
  exit(EXIT_FAILURE);
}

void handleErrors() {
  pid_t pid = fork();

  if (pid == -1) {
    messageError("Fork failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    // handle catchable errors
    std::set_terminate(exceptionHandler);
    return;
  } else {
    // handle uncatchable errors
    int status;
    waitpid(pid, &status, 0);

    if (WIFSIGNALED(status)) {
      // abnormal termination with a signal
      hlog::dumpErrorToFile("Abnormal termination with a signal",
                            WIFEXITED(status) ? WEXITSTATUS(status) : -1,
                            WTERMSIG(status));
      std::cerr << "Abnormal termination with signal " << WTERMSIG(status)
                << "\n";
      exit(WTERMSIG(status));
    } else if (WIFEXITED(status)) {
      // normal exit
      exit(WEXITSTATUS(status));
    } else {
      // abnormal exit
      hlog::dumpErrorToFile("Abnormal termination");
      std::cerr << "Abnormal termination with status " << WEXITSTATUS(status)
                << "\n";
      exit(WEXITSTATUS(status));
    }
  }

  exit(0);
}

void parseCommandLineArguments(int argc, char* args[]) {
  auto result = parseFlexer(argc, args);

  if (result.count("project-root")) {
    clc::projectRoot = result["project-root"].as<std::string>();
    if (!std::filesystem::exists(clc::projectRoot)) {
      messageError("Directory does not exist: " + clc::projectRoot);
    }
  }

  if (result.count("include")) {
    clc::include = result["include"].as<std::vector<std::string>>();
    messageErrorIf(clc::include.empty(),
                   "No file extensions provided with --include");
  }

  if (result.count("server-ip")) {
    clc::serverIp = result["server-ip"].as<std::string>();
  }

  if (result.count("port")) {
    clc::port = result["port"].as<size_t>();
  }
  if (result.count("client")) {
    clc::client = true;
  }
  if (result.count("server")) {
    clc::server = true;
  }
  messageErrorIf(clc::client && clc::server,
                 "Flexer cannot be client and server at the same time");
}
