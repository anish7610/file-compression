#ifndef LZ77_HPP
#define LZ77_HPP

#include "compression.hpp"
#include <cstddef>

class LZ77Compression : public Compression {
public:
    // constructor: windowSize (search buffer), lookaheadSize (max match length)
    LZ77Compression(size_t windowSize = 4096, size_t lookaheadSize = 18)
        : window(windowSize), lookahead(lookaheadSize) {}

    std::vector<unsigned char> compress(const std::vector<unsigned char>& data) override;
    std::vector<unsigned char> decompress(const std::vector<unsigned char>& data) override;

private:
    size_t window;
    size_t lookahead;
};

#endif // LZ77_HPP
