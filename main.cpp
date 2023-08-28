#include "Hasher.hpp"
#include "text.hpp"
#include "ThreadPool.hpp"
#include <unordered_set>
#include <experimental/filesystem>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include "simdjson.h"

using namespace simdjson;
namespace fs = std::experimental::filesystem;

constexpr int NUM_WORKERS = 2;

void processFile(const std::string &filePath, const std::string &outputDir,  std::unordered_set<std::string> &fullProcessedHashes, std::unordered_set<std::string> &partProcessedHashes, ThreadPool &pool){
    std::cout << "\nProcessing file: " << filePath << std::endl;    
    Hasher hasher(5, 100, 10, 10);
    Hasher hasher(5, 100, 10, 10);
    std::vector<std::string> outputLines;
    Hasher hasher(5, 100, 10, 10);    
    std::vector<std::string> outputLines;

    ondemand::parser parser;
    padded_string json = padded_string::load(filePath);    
    ondemand::document_stream docs = parser.iterate_many(json);

    std::vector<text> myTexts;    
    std::vector<std::future<void>> futures;
    std::cout << "start cal hash..." << std::endl;
    for (auto doc : docs) {
        std::string_view res;
        // todo handle error
        auto error = doc["text"].get(res);
        myTexts.emplace_back(std::string(res));
    }

    std::vector<std::future<void>> futures;
    for(text& myText: myTexts) {
        futures.push_back(pool.enqueue([&hasher, &myText]() {
            hasher.apply(myText);
        }));
    }

    std::cout << "wait cal hash..." << std::endl;
    for (auto& future : futures) {
        future.get();
    }

    std::cout << "start check dedup..." << std::endl;
    std::vector<std::string> outputLines;
    size_t duplicatedCount = 0;
    int i=0;
    for(auto myText : myTexts){        
        bool isDuplicate = false;        
        for (const std::string& hashValue : myText.getHashes()) {
            if (fullProcessedHashes.find(hashValue) != fullProcessedHashes.end() || 
                partProcessedHashes.find(hashValue) != partProcessedHashes.end()
            ) {
                isDuplicate = true;
                duplicatedCount++;
                break;
            }
        }

        if (!isDuplicate) {
            for (const std::string& hashValue : myText.getHashes()) {
                partProcessedHashes.insert(hashValue);
            }
            outputLines.push_back(myText.getContent());
        }
        if (i % 5000 == 0) {
            std::cout << "dedup:    \r" << i << std::flush;
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
    myTexts.shrink_to_fit();
}


void processFiles(int start, int end, const std::string& inputDir, const std::string& outputDir, const std::string& processedHashesDir) {    
    // blacklistをロード
    std::unordered_set<std::string> processedHashes;
    for (int i = 0; i <= 119; ++i) {
        std::string processedHashesFile = processedHashesDir + "/" + std::to_string(i) + ".txt";
        if(fs::exists(processedHashesFile)) {
            std::ifstream hashesFile(processedHashesFile);
            std::cout << "load blacklist file..." << processedHashesFile << std::endl;
            if (hashesFile.is_open()) {
                std::string hash;
                while (hashesFile >> hash) {
                    processedHashes.insert(hash);
                }
                hashesFile.close();
            }
        }
    }

    ThreadPool pool(NUM_WORKERS);
    std::cout << "worker..." << NUM_WORKERS << std::endl;
    for (int i = start; i <= end; ++i) {
        std::unordered_set<std::string> partProcessedHashes;
        std::string filePath = inputDir + "/" + std::to_string(i) + ".jsonl";
        if(!fs::exists(filePath)) {
            std::cout << filePath <<"not found... skip..." << std::endl;
            continue;
        }
        processFile(
            filePath, 
            outputDir, 
            std::ref(processedHashes),
            std::ref(partProcessedHashes), 
            pool
        );

        // processedHashes を更新
        std::string processedHashesFile = processedHashesDir + "/" + std::to_string(i) + ".txt";
        std::ofstream hashesFile(processedHashesFile);
        for (const std::string& hash : partProcessedHashes) {
            hashesFile << hash << std::endl;
        }
        hashesFile.close();
        std::cout << "save blacklist file..." << processedHashesFile << std::endl;

        // blacklistをmerge
        for (const std::string& hash : partProcessedHashes) {
            processedHashes.insert(hash);            
        }
        partProcessedHashes.clear();        
    }
}

int main(int argc, char *argv[]){    
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <start> <end> <inputDir> <outputDir> <processedHashesDir>" << std::endl;
        return 1;
    }
    int start = std::stoi(argv[1]);
    int end = std::stoi(argv[2]);
    std::string inputDir = argv[3];
    std::string outputDir = argv[4];
    std::string processedHashesDir = argv[5];

    processFiles(start, end, inputDir, outputDir, processedHashesDir);
    return 0;
}
