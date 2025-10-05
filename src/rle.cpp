#include "rle.hpp"

std::vector<unsigned char> RLECompression::compress(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> output;
    if (data.empty()) return output;

    unsigned char prev = data[0];
    int count = 1;

    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] == prev && count < 255) {
            count++;
        } else {
            output.push_back(prev);
            output.push_back(static_cast<unsigned char>(count));
            prev = data[i];
            count = 1;
        }
    }
    output.push_back(prev);
    output.push_back(static_cast<unsigned char>(count));
    return output;
}

std::vector<unsigned char> RLECompression::decompress(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> output;
    for (size_t i = 0; i < data.size(); i += 2) {
        unsigned char value = data[i];
        int count = data[i+1];
        for (int j = 0; j < count; ++j) {
            output.push_back(value);
        }
    }
    return output;
}
