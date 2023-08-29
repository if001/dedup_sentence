#include "dedup.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;
// #include <experimental/filesystem>
// namespace fs = std::experimental::filesystem;


std::unordered_set<std::string> dedup(std::vector<text> &myTexts, const std::unordered_set<std::string> blackList) {
    std::unordered_set<std::string> newBlackList;
     for(auto &myText : myTexts){
        if(myText.isDuplicate) {
            continue;
        }
        for (const std::string& hashValue : myText.getHashes()) {
            if (blackList.find(hashValue) != blackList.end() ||
                newBlackList.find(hashValue) != newBlackList.end()) {
                myText.isDuplicate = true;                
                break;
            }
        }            
        for (const std::string& hashValue : myText.getHashes()) {
            newBlackList.insert(hashValue);
        }
    }
    return newBlackList;
}

std::unordered_set<std::string> dedupByBlackListFile(std::vector<text> &myTexts, const std::string blackListFilePath) {
    std::unordered_set<std::string> blackList;
    if(fs::exists(blackListFilePath)) {
        // std::cout << blackListFilePath << std::endl;
        std::ifstream blackListFile(blackListFilePath);                        
        if (blackListFile.is_open()) {
            std::string hash;
            while (blackListFile >> hash) {
                blackList.insert(hash);
            }
            blackListFile.close();
        }
        // std::cout << blackList.size() << std::endl;
    }
    std::unordered_set<std::string> newBlackList = dedup(myTexts, blackList);
    blackList.clear();    
    return newBlackList;
}