#include <unordered_set>
#include "text.hpp"

std::unordered_set<std::string> dedup(std::vector<text> &myTexts, const std::unordered_set<std::string> blackList);

std::unordered_set<std::string> dedupByBlackListFile(std::vector<text> &myTexts, const std::string blackListFilePath);