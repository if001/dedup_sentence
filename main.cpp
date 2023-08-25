#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include "text.hpp"
#include "Hasher.hpp"
#include <experimental/filesystem>  // GCC 9

using json = nlohmann::json;
// namespace fs = std::filesystem;
namespace fs = std::experimental::filesystem;


void processFiles(const std::string& inputDir, const std::string& outputDir) {
    Hasher hasher(3, 30, 10, 3);
    std::unordered_set<std::string> processedHashes;

    for (const auto& file : fs::directory_iterator(inputDir)) {
        if (file.path().extension() == ".jsonl") {
            std::cout << "file: " << file.path() << std::endl;
            std::string outputFileName = outputDir + "/" + file.path().filename().string();
            std::vector<std::string> outputLines;

            std::ifstream inFile(file.path());
            std::string line;

            while (std::getline(inFile, line)) {
                json jsonObj = json::parse(line);
                std::string textContent = jsonObj["text"].get<std::string>();

                text myText(textContent);
                hasher.apply(myText);

                bool isDuplicate = false;
                for (const std::string& hashValue : myText.getHashes()) {
                    if (processedHashes.find(hashValue) != processedHashes.end()) {
                        isDuplicate = true;
                        break;
                    }
                }

                if (!isDuplicate) {
                    for (const std::string& hashValue : myText.getHashes()) {
                        processedHashes.insert(hashValue);
                    }
                    outputLines.push_back(line);
                }
            }

            inFile.close();

            std::ofstream outFile(outputFileName);
            for (const auto& line : outputLines) {
                outFile << line << std::endl;
            }
            outFile.close();
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input directory> <output directory>" << std::endl;
        return 1;
    }

    std::string inputDir = argv[1];
    std::string outputDir = argv[2];

    processFiles(inputDir, outputDir);
    return 0;
}