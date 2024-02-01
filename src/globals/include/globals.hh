
#pragma once
#include <stddef.h>
#include <string>
#include <vector>

// command line config
namespace clc {
extern bool silent;
///--wilent
extern bool wsilent;
///--isilent
extern bool isilent;
///--psilent
extern bool psilent;
} // namespace clc

// harm stat
namespace hs {

/// The name of the current "execution", --name
extern std::string name;
} // namespace hs
