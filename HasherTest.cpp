#include <gtest/gtest.h>

#include "Hasher.hpp"
#include "text.hpp"
#include "dedup.hpp"

TEST(SimpleTest, Show) {
    Hasher hasher(5, 100, 10, 10);    
    text myText("おはようございます");
    hasher.apply(myText);
    std::cout << "text: " << myText.getContent() << "\n";
    for (std::string hash : myText.getHashes()) {
        std::cout << hash << "\n";
    }
}



bool check(text text1, text text2) {
  bool same = false;
  std::vector<std::string> hashes = text1.getHashes();
  for(auto h2 : text2.getHashes()) {
    if(std::find(hashes.begin(), hashes.end(), h2) != hashes.end()) {
      same = true;
      break;
    }
  }
  return same;  
}


TEST(SimpleTest, BetweenText) {
    Hasher hasher(5, 100, 10, 10);    
    text myText0("吾輩は猫である。名前はまだ無い。どこで生まれたかとんと見当がつかぬ。");
    hasher.apply(myText0);
    std::cout << "text: " << myText0.getContent() << std::endl;
    for (std::string hash : myText0.getHashes()) {
        std::cout << hash << std::endl;
    }
    std::cout << "-----------" << std::endl;

    text myText1("吾輩は猫である。名前はまだ無い。どこで生まれたかとんと見当がつかぬ。");
    hasher.apply(myText1);
    std::cout << "text: " << myText1.getContent() << std::endl;
    for (std::string hash : myText1.getHashes()) {
        std::cout << hash << std::endl;
    }
    std::cout << "-----------" << std::endl;

    text myText2("吾輩は猫である。");
    hasher.apply(myText2);
    std::cout << "text: " << myText2.getContent() << std::endl;
    for (std::string hash : myText2.getHashes()) {
        std::cout << hash << std::endl;
    }
  std::cout << "-----------" << std::endl;

    text myText3("どこで生まれたかとんと見当がつかぬ。");
    hasher.apply(myText3);
    std::cout << "text: " << myText3.getContent() << std::endl;
    for (std::string hash : myText3.getHashes()) {
        std::cout << hash << std::endl;
    }

    std::cout << "text0_text1 " << check(myText0, myText1) << std::endl;
    std::cout << "text1_text2 " << check(myText1, myText2) << std::endl;
    std::cout << "text2_text3 " << check(myText2, myText3) << std::endl;
    std::cout << "text3_text1 " << check(myText3, myText1) << std::endl;    
}


TEST(DEDUP, simple) {
  Hasher hasher(5, 100, 10, 10);    
  // text myText("吾輩は猫である。名前はまだ無い。どこで生まれたかとんと見当がつかぬ。");
  text myText("ファイル3ファイル3ファイル3ファイル3ファイル3");
  hasher.apply(myText);    
  for (auto h: myText.getHashes()){
    std::cout << h << std::endl;
  }
  std::cout << "-----------------" << std::endl;
  std::vector<text> myTexts = {myText};
  auto hash2 = dedupByBlackListFile(myTexts, "./blacklists/3.txt");
  for (auto h: hash2){
    std::cout << h << std::endl;
  }

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}