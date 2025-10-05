#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include "compression.hpp"
#include <vector>

class HuffmanCompression : public Compression {
public:
    std::vector<unsigned char> compress(const std::vector<unsigned char>& data) override;
    std::vector<unsigned char> decompress(const std::vector<unsigned char>& data) override;
    ~HuffmanCompression() override = default;
};

#endif // HUFFMAN_HPP
