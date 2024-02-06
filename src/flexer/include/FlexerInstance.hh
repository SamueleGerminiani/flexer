
#pragma once
#include <cstdlib>
#include <string>

namespace flexer {
struct FlexerInstance {
  std::string id;
  std::string text;
  size_t startLine;
  size_t endLine;
  std::string fileName;
};
}  // namespace flexer
