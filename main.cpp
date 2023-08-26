#include "Hasher.hpp"
#include "text.hpp"
#include <unordered_set>
#include <experimental/filesystem>
#include <vector>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "simdjson.h"
using namespace simdjson;
namespace fs = std::experimental::filesystem;

void processFile(const std::string &filePath, const std::string &outputDir,  std::unordered_set<std::string> &processedHashes){
    std::cout << "\nProcessing file: " << filePath << std::endl;    
    Hasher hasher(5, 100, 10, 10);
    std::vector<std::string> outputLines;

    ondemand::parser parser;
    padded_string json = padded_string::load(filePath);    
    ondemand::document_stream docs = parser.iterate_many(json);
    size_t duplicatedCount = 0;
    int i=0;
    for (auto doc : docs) {
        std::string textContent;
        std::string_view res;
        auto error = doc["text"].get(res);
        if (!error) {
            std::string textContent = std::string(res);
            text myText(textContent);
            hasher.apply(myText);

            bool isDuplicate = false;
            for (const std::string& hashValue : myText.getHashes()) {
                if (processedHashes.find(hashValue) != processedHashes.end()) {
                    isDuplicate = true;
                    duplicatedCount++;
                    break;
                }
            }

            if (!isDuplicate) {
                for (const std::string& hashValue : myText.getHashes()) {
                    processedHashes.insert(hashValue);
                }
                outputLines.push_back(textContent);
            }
        }
        if (i % 5000 == 0) {
            std::cout << "    \r" << i << std::flush;
        }        
        i++;
    }
    std::cout << "\nDuplicated: " << duplicatedCount << std::endl;

    std::string outputFileName = outputDir + "/" + fs::path(filePath).filename().string();
    std::ofstream outFile(outputFileName);
    for (const auto &line : outputLines) {
        nlohmann::json li;
        li["text"]= line;
        outFile << li.dump() << std::endl;
    }
    outFile.close();

}


void processFiles(int start, int end, const std::string& inputDir, const std::string& outputDir, const std::string& processedHashesFile) {
    std::unordered_set<std::string> processedHashes;
    if (!processedHashesFile.empty()) {
        std::ifstream hashesFile(processedHashesFile);
        std::cout << 'load blacklist...' << processedHashesFile << std::endl;
        if (hashesFile.is_open()) {
            std::string hash;
            while (hashesFile >> hash) {
                processedHashes.insert(hash);
            }
            hashesFile.close();
        }
    }

    for (int i = start; i <= end; ++i) {
        std::string filePath = inputDir + "/" + std::to_string(i) + ".jsonl";
        processFile(filePath, outputDir, std::ref(processedHashes));

        // processedHashes を更新        
        std::ofstream hashesFile(processedHashesFile);
        std::cout << 'save blacklist...' << processedHashesFile << std::endl;
        for (const std::string& hash : processedHashes) {
            hashesFile << hash << std::endl;
        }
        hashesFile.close();
    }
}

int main(int argc, char *argv[]){    
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <start> <end> <inputDir> <outputDir> [<processedHashesFile>]" << std::endl;
        return 1;
    }
    int start = std::stoi(argv[1]);
    int end = std::stoi(argv[2]);
    std::string inputDir = argv[3];
    std::string outputDir = argv[4];
    std::string processedHashesFile = argc > 5 ? argv[5] : "";

    processFiles(start, end, inputDir, outputDir, processedHashesFile);
    return 0;
}
