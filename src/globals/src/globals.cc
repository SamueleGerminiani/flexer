#include "globals.hh"

#include <limits>
#include <thread>

// @start-flexer[1.1]
namespace clc {
bool silent = false;
bool wsilent = false;
bool isilent = false;
bool psilent = false;
std::string inputDir;
std::string inputFile;
std::vector<std::string> extensions;
}  // namespace clc
// @end-flexer

namespace hs {
std::string name = "";
}  // namespace hs
