#ifndef RLE_HPP
#define RLE_HPP

#include "compression.hpp"

class RLECompression : public Compression {
public:
    std::vector<unsigned char> compress(const std::vector<unsigned char>& data) override;
    std::vector<unsigned char> decompress(const std::vector<unsigned char>& data) override;
};

#endif
