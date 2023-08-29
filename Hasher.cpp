#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

#include "Hasher.hpp"
#include "text.hpp"
#include "./smhasher/src/MurmurHash3.h"
#include "Hasher.hpp"
#include "text.hpp"

size_t Hasher::hash(const std::string &s, uint32_t seed) const {
    uint64_t output[2];
    MurmurHash3_x64_128(s.data(), s.length(), seed, &output);
    return output[0];
}

Hasher::Hasher(size_t n, size_t n_minhash, size_t n_bucket, size_t bucket_size) 
    : N(n), N_MINHASH(n_minhash), N_BUCKET(n_bucket), BUCKET_SIZE(bucket_size) {
    if(N_MINHASH != N_BUCKET * BUCKET_SIZE) {
        std::cerr << "Error: N_MINHASH does not match N_BUCKET * BUCKET_SIZE." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

// text: 吾輩は猫である。名前はまだ無い。どこで生まれたかとんと見当がつかぬ。
// 0+089909ae452c3755f5b25bdea58221f80f76e435
// 1+38906d6ecb902eedcc5e00eefa6ad4bf8b75b56c
// 2+3f4226e0bdb80f2854b3ebaeb6346faa7f750ebf
// 3+ba8e516918e951c236a665025179f0400943344e
// 4+84f86680b3355711cb836516ad5f5bd1e1228b4a
// 5+4b9640dbb0e776b03d241ef5419f10d3736d6e9a
// 6+f57c26f373a032b551692d371a9831f293b7353c
// 7+0fcb198752db9673e015fd19906332794078fa6f
// 8+4a8d60c95672184c55176c65b33b8c33d73e6a06
// 9+a30927be26feb52b492682b9ae14ea3514b8d825
// 
// text: 吾輩は猫である。
// 0+b2a3d2af945c22da53b5d66e39056d70f5cee435
// 1+38909f0f2ce8e5065d07b5b5fa6ab4d25a614a14
// 2+06491107bdb80f28af7dc598d3fd6faa60c2da7f
// 3+ba8e7cfd982c7ecbb092bd8939c074cc1b13f5fb
// 4+17020c20b3355711a70b65164ebb5bd18c2a8b4a
// 5+a65e40db19e33dc93d248ba3419fc649df30afb1
// 6+06224685f13f027c45102d37ad14b21d7381ee5f
// 7+0fcbc4c71f35eb9a3806f18b9063a89dbdd5a3d2
// 8+4a8dd10859be184c2b0e0e27b33bd683f5c76a06
// 9+62038accbc32164e6001d122ae14ea351d060dd0


void Hasher::apply(text &txt) const {
    std::string s = txt.getContent();
    std::vector<size_t> minHashes(N_MINHASH, std::numeric_limits<size_t>::max());

    for (size_t i = 0; i <= s.length() - N; ++i) {
        for (size_t j = 0; j < N_MINHASH; ++j) {
            minHashes[j] = std::min(minHashes[j], hash(s.substr(i, N), j));
        }
    }

    std::vector<std::string> bucketedHashes;
    for (size_t i = 0; i < N_BUCKET; ++i) {
        std::stringstream ss;
        for (size_t j = 0; j < BUCKET_SIZE; ++j) {
            ss << std::hex << std::setw(4) << std::setfill('0') << (minHashes[i * BUCKET_SIZE + j] & 0xFFFF);
        }        
        bucketedHashes.push_back(std::to_string(i) + "+" + ss.str());
    }
    
    txt.setHashes(bucketedHashes);
}