#include "globals.hh"

#include <limits>
#include <thread>

namespace clc {
bool silent = false;
bool wsilent = false;
bool isilent = false;
bool psilent = false;
std::string projectRoot;
std::vector<std::string> include;
std::string serverIp;
size_t port;
bool client;
bool server;
}  // namespace clc

namespace hs {
std::string name = "";
}  // namespace hs
