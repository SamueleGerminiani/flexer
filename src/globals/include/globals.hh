
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

extern std::string projectRoot;
extern std::vector<std::string> include;
extern std::string serverIp;
extern size_t port;
extern bool client;
extern bool server;
}  // namespace clc

// harm stat
namespace hs {}  // namespace hs
