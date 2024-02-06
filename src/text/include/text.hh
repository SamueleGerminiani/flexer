#pragma once

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "FlexerInstance.hh"
#include "globals.hh"
#include "message.hh"

namespace flexer {

/// @brief Extract all the flexer instances from the given file
inline std::vector<FlexerInstance> extractFlexerInstances(
    const std::string& filePath) {
  std::vector<FlexerInstance> flexerInstances;

  // Read the content of the file
  std::ifstream file(filePath);
  if (!file.is_open()) {
    messageError("Failed to open file: " + filePath);
    return flexerInstances;
  }

  const std::string startTag = "@start-flexer";
  const std::string endTag = "@end-flexer";
  std::string line;
  std::string id;
  std::string text;
  bool insideFlexer = false;
  size_t currLineNumber = 0;
  size_t startTagLineNumber = 0;

  while (std::getline(file, line)) {
    ++currLineNumber;

    size_t startIdx = line.find(startTag);
    size_t endIdx = line.find(endTag);

    messageErrorIf(startIdx != std::string::npos && endIdx != std::string::npos,
                   "Both start and end tags found on the same line at line " +
                       std::to_string(currLineNumber) +
                       " in file: " + filePath);
    messageErrorIf(
        insideFlexer && startIdx != std::string::npos,
        "Unexpected start tag found inside a flexer instance at line " +
            std::to_string(currLineNumber) + " in file: " + filePath);
    messageErrorIf(!insideFlexer && endIdx != std::string::npos,
                   "Unexpected end tag found before a corresponding "
                   "@start-flexer at line " +
                       std::to_string(currLineNumber) +
                       " in file: " + filePath);
    messageErrorIf(!insideFlexer && endIdx != std::string::npos,
                   "Unexpected end tag found at line " +
                       std::to_string(currLineNumber) +
                       " in file: " + filePath);

    if (startIdx != std::string::npos) {
      insideFlexer = true;
      id.clear();
      text.clear();
      startTagLineNumber = currLineNumber + 1;

      size_t idStartIdx = line.find('[');
      size_t idEndIdx = line.find(']');
      messageErrorIf(
          idEndIdx == std::string::npos || idStartIdx == std::string::npos,
          "Error when parsing flexer ID at line " +
              std::to_string(currLineNumber) + " in file: " + filePath);
      id = line.substr(idStartIdx + 1, idEndIdx - idStartIdx - 1);
      messageErrorIf(id.empty(), "Empty flexer ID found at line " +
                                     std::to_string(currLineNumber) +
                                     " in file: " + filePath);

      continue;
    }

    if (endIdx != std::string::npos && insideFlexer) {
      flexerInstances.push_back(
          {id, text, startTagLineNumber, currLineNumber - 1, filePath});
      insideFlexer = false;
      continue;
    }

    if (insideFlexer) {
      text += line + "\n";
    }
  }

  // Check for an incomplete flexer instance at the end of the file
  messageErrorIf(
      insideFlexer,
      "Unmatched end tag after reaching the end of the file in file: " +
          filePath);

  return flexerInstances;
}

///@brief Extract all the flexer instances from the given files
inline std::vector<FlexerInstance> extractFlexerInstances(
    const std::vector<std::string>& filePath) {
  std::vector<FlexerInstance> flexerInstances;
  for (const auto& file : filePath) {
    auto instances = extractFlexerInstances(file);
    flexerInstances.insert(flexerInstances.end(), instances.begin(),
                           instances.end());
  }

  std::unordered_map<std::string, size_t> occurrences;

  bool duplicateIdsFound = false;
  for (const auto& instance : flexerInstances) {
    occurrences[instance.id]++;
    if (occurrences.at(instance.id) > 1) {
      duplicateIdsFound = true;
    }
  }

  if (duplicateIdsFound) {
    std::string duplicateIdsStr = "\n";
    for (const auto& instance : flexerInstances) {
      if (occurrences.at(instance.id) > 1) {
        duplicateIdsStr += "\t" + instance.id + " found in file:\n";
        duplicateIdsStr += "\t\t" + instance.fileName + " " + "lines " +
                           std::to_string(instance.startLine) + " to " +
                           std::to_string(instance.endLine) + "\n";
        duplicateIdsStr += "\n";
      }
    }
    messageError("Duplicate flexer IDs found: " + duplicateIdsStr);
  }

  return flexerInstances;
}

///@brief Organize the flexer instances by file
inline std::unordered_map<std::string, std::vector<FlexerInstance>>
organizeInstances(const std::vector<FlexerInstance>& instances) {
  std::unordered_map<std::string, std::vector<FlexerInstance>> resultMap;

  for (const auto& flexerInstance : instances) {
    resultMap[flexerInstance.fileName].push_back(flexerInstance);
  }

  for (auto& fileInstances : resultMap) {
    std::sort(fileInstances.second.begin(), fileInstances.second.end(),
              [](const FlexerInstance& a, const FlexerInstance& b) {
                return a.startLine < b.startLine;
              });
  }

  return resultMap;
}

///@brief Substitute the flexer instances in the file
inline std::string subtituteFlexerInstances(
    std::string fileName, const std::vector<FlexerInstance>& subInstances) {
  messageErrorIf(subInstances.empty(), "No flexer subInstances provided");

  // check that all subInstances are from the same file
  for (const auto& instance : subInstances) {
    messageErrorIf(instance.fileName != fileName,
                   "Flexer subInstances from different files provided");
  }

  // Read the content of the file
  std::ifstream file(fileName);
  if (!file.is_open()) {
    messageError("Failed to open file: " + fileName);
    return "";
  }

  std::string line;
  // current line of the file
  size_t currLineNumber = 0;
  // current index of the subInstances being substituted
  size_t subIndex = 0;
  // number of lines to skip when encountering a flexer instance: the original
  // text of the instance is not included in the file
  size_t skipLines = 0;
  // the text of the file with the subInstances substituted
  std::stringstream result;

  while (std::getline(file, line)) {
    ++currLineNumber;
    if (subInstances[subIndex].startLine == currLineNumber &&
        subIndex < subInstances.size()) {
      // substitute the flexer instance
      result << subInstances[subIndex].text;
      skipLines =
          subInstances[subIndex].endLine - subInstances[subIndex].startLine;
      ++subIndex;
    } else if (skipLines > 0) {
      // skip the lines of the flexer instance
      --skipLines;
    } else {
      // copy the line as is
      result << line << "\n";
    }
  }

  file.close();

  messageErrorIf(
      subIndex != subInstances.size(),
      "Not all flexer subInstances were substituted in file: " + fileName);

  return result.str();
}

inline std::unordered_map<std::string, std::vector<FlexerInstance>>
generateSubInstances(
    const std::unordered_map<std::string, std::vector<FlexerInstance>>&
        toBeSubstituted,
    const std::unordered_map<std::string, std::vector<FlexerInstance>>&
        subtitutions) {
  std::unordered_map<std::string, std::vector<FlexerInstance>> resultMap;

  for (const auto& [fileName, instances] : toBeSubstituted) {
    // initialize the result map
    resultMap[fileName];
    // if the file has subtitutions
    if (subtitutions.count(fileName)) {
      // get the subtitutions for the file
      auto& subInstances = subtitutions.at(fileName);
      // for each instance potentially to be substituted
      for (const auto& tbsInstance : instances) {
        // find if the instance has a subtitution
        auto last =
            std::find_if(subInstances.begin(), subInstances.end(),
                         [&tbsInstance](const FlexerInstance& subInstance) {
                           return subInstance.id == tbsInstance.id;
                         });

        if (last != subInstances.end()) {
          // if the instance has a subtitution, load the subtitution text
          resultMap[fileName].push_back(tbsInstance);
          resultMap.at(fileName).back().text = last->text;

        } else {
          // if the instance does not have a subtitution, keep the original text
          resultMap[fileName].push_back(tbsInstance);
        }
      }
    } else {
      // if the file does not have subtitutions, fill with the original
      // instances
      resultMap[fileName] = instances;
    }
  }

  return resultMap;
}


}  // namespace flexer
