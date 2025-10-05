#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

#include <vector>
#include <string>

class Compression {
public:
    virtual ~Compression() = default;

    // Compress input data into output vector
    virtual std::vector<unsigned char> compress(const std::vector<unsigned char>& data) = 0;

    // Decompress input data into output vector
    virtual std::vector<unsigned char> decompress(const std::vector<unsigned char>& data) = 0;
};

#endif
