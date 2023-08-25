#include "Hasher.hpp"
#include "text.hpp"
#include <unordered_set>
#include <experimental/filesystem>
#include <thread>
#include <vector>
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>
#include "simdjson.h"
using namespace simdjson;
namespace fs = std::experimental::filesystem;

std::mutex mtx; // 重複判定

std::mutex progress_mtx; // プログレスバー

void displayProgressBar(size_t progress, size_t total){
    const size_t barWidth = 60;
    float percentage = static_cast<float>(progress) / total;

    progress_mtx.lock();
    std::cout << "[";
    size_t pos = static_cast<size_t>(barWidth * percentage);
    for (size_t i = 0; i < barWidth; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(percentage * 100.0) << " %\r";
    std::cout.flush();
    progress_mtx.unlock();
}

void processFile(const std::string &filePath, const std::string &outputDir, Hasher &hasher, std::unordered_set<std::string> &processedHashes){
    std::cout << "Processing file: " << filePath << std::endl;

    std::vector<std::string> outputLines;

    ondemand::parser parser;
    padded_string json = padded_string::load(filePath);    
    ondemand::document_stream docs = parser.iterate_many(json);
    static size_t progress = 0;
    size_t duplicatedCount = 0;

    for (auto doc : docs) {
        std::string textContent;        
        std::string_view res;
        auto error = doc["text"].get(res);
        if (!error) {
            std::string textContent = std::string(res);
            text myText(textContent);
            hasher.apply(myText);

            mtx.lock();
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
            mtx.unlock();
        }
        displayProgressBar(++progress, fs::file_size(filePath));
    }
    std::cout << "Duplicated texts in " << filePath << ": " << duplicatedCount << std::endl;

    std::string outputFileName = outputDir + "/" + fs::path(filePath).filename().string();
    std::ofstream outFile(outputFileName);
    for (const auto &line : outputLines) {
        nlohmann::json li;
        li["text"]= line;
        outFile << li.dump() << std::endl;
    }
    outFile.close();
}

void processFiles(const std::string &inputDir, const std::string &outputDir){
    Hasher hasher(5, 200, 20, 10);
    std::unordered_set<std::string> processedHashes;

    std::vector<std::thread> threads;

    for (const auto &file : fs::directory_iterator(inputDir)){
        if (file.path().extension() == ".jsonl"){
            threads.emplace_back(processFile, file.path().string(), outputDir, std::ref(hasher), std::ref(processedHashes));
        }
    }

    for (std::thread &th : threads){
        if (th.joinable()){
            th.join();
        }
    }
}

int main(int argc, char *argv[]){    
    if (argc < 3){
        std::cerr << "Usage: " << argv[0] << " <input directory> <output directory>" << std::endl;
        return 1;
    }

    std::string inputDir = argv[1];
    std::string outputDir = argv[2];

    processFiles(inputDir, outputDir);
    return 0;
}
