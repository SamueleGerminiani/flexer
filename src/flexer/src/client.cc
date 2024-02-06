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
#include "flexer.hh"
#include "flexerIcon.hh"
#include "globals.hh"
#include "message.hh"

/// @brief handle all the command line arguments
static void parseCommandLineArguments(int argc, char* args[]);

/// @brief handles all the unhandled exceptions
void exceptionHandler();

/// @brief handles all the unhandled errors
void handleErrors();

using namespace flexer;

int main(int arg, char* argv[]) {
#ifndef DEBUG
  handleErrors();
#endif

  // enforce deterministic rand
  srand(1);

  // print welcome message
  std::cout << getIcon() << "\n";

  parseCommandLineArguments(arg, argv);

  // find all the files with the given extensions------------
  std::vector<std::string> inFiles = findFiles();

  auto instances = extractFlexerInstances(inFiles);
  messageErrorIf(instances.empty(), "No flexer instances found");

  // debug
  // for (const auto& [id, text, startLine, endLine, fileName] : instances) {
  //   std::cout << "ID: " << id << "\n";
  //   std::cout << "Text: \n" << text << "\n";
  //   std::cout << "Start line: " << startLine << "\n";
  //   std::cout << "End line: " << endLine << "\n";
  //   std::cout << "File: " << fileName << "\n";
  // }

  std::unordered_map<std::string, std::vector<FlexerInstance>> toBeSubstituted =
      organizeInstances(instances);

  std::unordered_map<std::string, std::vector<FlexerInstance>> subtitutions =
      toBeSubstituted;

  size_t i = 0;
  for (auto& [file, instances] : subtitutions) {
    for (auto& instance : instances) {
      instance.text = "substitution_" + std::to_string(i++) + "\n";
    }
  }

  std::unordered_map<std::string, std::vector<FlexerInstance>>
      fileToSubInstances = generateSubInstances(toBeSubstituted, subtitutions);

  std::cout << "=============================================================="
            << "\n";
  // print file to sub instances
  for (const auto& [fileName, subInstances] : fileToSubInstances) {
    std::cout << "File: " << fileName << "\n";
    for (const auto& instance : subInstances) {
      std::cout << "ID: " << instance.id << "\n";
      std::cout << "Text: " << instance.text << "\n";
    }
  }
  std::cout << "==========================================================="
            << "\n";

  std::cout << "---------------------------------------------------------------"
               "-------"
            << "\n";
  for (const auto& [fileName, subInstances] : fileToSubInstances) {
    std::cout << "File: " << fileName << "\n";
    std::cout << subtituteFlexerInstances(fileName, subInstances) << "\n";
  }

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

  if (result.count("dir")) {
    clc::inputDir = result["dir"].as<std::string>();
    if (!std::filesystem::exists(clc::inputDir)) {
      messageError("Directory does not exist: " + clc::inputDir);
    }
  }
  if (result.count("file")) {
    clc::inputFile = result["file"].as<std::string>();
    if (!std::filesystem::exists(clc::inputFile)) {
      messageError("File does not exist: " + clc::inputFile);
    }
  }
  if (result.count("ss")) {
    clc::extensions = result["ss"].as<std::vector<std::string>>();
    messageErrorIf(clc::extensions.empty(), "No suffixes provided");
  }
}
