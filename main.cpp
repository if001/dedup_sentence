#include <unordered_set>
#include <experimental/filesystem>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include "simdjson.h"
#include "Hasher.hpp"
#include "text.hpp"
#include "ThreadPool.hpp"
#include "dedup.hpp"

using namespace simdjson;
namespace fs = std::experimental::filesystem;

constexpr int NUM_WORKERS = 2;



std::unordered_set<std::string> processFile(
    const std::string &filePath, const std::string &outputDir, 
    const std::string &blackListDir, ThreadPool &pool, const int max_blacklist_idx){
    std::cout << "\nProcessing file: " << filePath << std::endl;    

    ondemand::parser parser;
    padded_string json = padded_string::load(filePath);    
    ondemand::document_stream docs = parser.iterate_many(json);

    std::vector<text> myTexts;
    std::cout << "start cal hash..." << std::endl;
    for (auto doc : docs) {
        std::string_view res;
        // todo handle error
        auto error = doc["text"].get(res);
        myTexts.emplace_back(std::string(res));
    }

    Hasher hasher(5, 100, 10, 10);
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

    std::unordered_set<std::string> newBlackList;
   

    std::cout << "start check dedup other file..." << std::endl;
    std::unordered_set<std::string> outputLines;

    if(fs::is_empty(blackListDir)) {
        newBlackList = dedup(std::ref(myTexts), std::unordered_set<std::string>({}));
    }

    for (int i = 1; i < max_blacklist_idx; ++i) {
        std::string blackListFilePath = blackListDir + "/" + std::to_string(i) + ".txt";
        std::cout << blackListFilePath << std::endl;
        auto b = dedupByBlackListFile(std::ref(myTexts), blackListFilePath);
        newBlackList.insert(b.begin(), b.end());
        b.clear();
    }

    std::string outputFileName = outputDir + "/" + fs::path(filePath).filename().string();
    std::ofstream outFile(outputFileName);

    size_t duplicateCount = 0;
    for(auto myText : myTexts) {
        if(!myText.isDuplicate) {
            nlohmann::json li;
            li["text"] = myText.getContent();
            outFile << li.dump() << std::endl;            
        }
        if(myText.isDuplicate) {
            duplicateCount++;                        
        }
    }
    
    // for(auto myText : myTexts) {
    //     std::cout << myText.isDuplicate << " : " << myText.getContent() << std::endl;
    // }

    std::cout << "\nDuplicated: " << duplicateCount << "/" << myTexts.size() << std::endl;
    outFile.close();
    myTexts.shrink_to_fit();
    outputLines.clear();
    return newBlackList;
}


void processFiles(int start, int end, const std::string& inputDir, const std::string& outputDir, const std::string& blackListDir) {    
    ThreadPool pool(NUM_WORKERS);
    std::cout << "worker..." << NUM_WORKERS << std::endl;
    for (int i = start; i <= end; ++i) {        
        std::string filePath = inputDir + "/" + std::to_string(i) + ".jsonl";
        if(!fs::exists(filePath)) {
            std::cout << filePath <<"not found... skip..." << std::endl;
            continue;
        }

        std::unordered_set<std::string> blackList = processFile(            
            filePath, 
            outputDir,
            blackListDir,
            pool,
            i
        );

        // processedHashes を更新
        std::string blackListFilePath = blackListDir + "/" + std::to_string(i) + ".txt";
        std::ofstream blackListFile(blackListFilePath);
        for (const std::string& hash : blackList) {        
            blackListFile << hash << std::endl;
        }
        blackListFile.close();
        std::cout << "save blacklist file..." << blackListFilePath << std::endl;
        blackList.clear();        
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
    std::string blackListDir = argv[5];

    processFiles(start, end, inputDir, outputDir, blackListDir);
    return 0;
}
