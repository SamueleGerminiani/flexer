
#pragma once
#include <stddef.h>

#include <string>
#include <vector>

// command line config
namespace clc {
extern std::string inputDir;
extern std::string inputFile;
extern std::vector<std::string> extensions;
extern bool silent;
///--wilent
extern bool wsilent;
///--isilent
extern bool isilent;
///--psilent
extern bool psilent;
}  // namespace clc

// @start-flexer[1.2]
// harm stat
namespace hs {
/// The name of the current "execution", --name
extern std::string name;
}  // namespace hs
// @end-flexer
