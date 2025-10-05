#include "lz77.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>

using namespace std;

// compress: naive sliding-window search O(n * window) — simple and didactic
vector<unsigned char> LZ77Compression::compress(const vector<unsigned char>& data) {
    vector<unsigned char> out;
    size_t n = data.size();
    size_t i = 0;

    while (i < n) {
        size_t maxMatchLen = 0;
        size_t bestOffset = 0;

        size_t start = (i >= window) ? (i - window) : 0;
        size_t endSearch = i;

        // search backward for longest match
        for (size_t j = start; j < endSearch; ++j) {
            size_t k = 0;
            // compare bytes while within lookahead and input bounds
            while (k < lookahead && i + k < n && data[j + k] == data[i + k]) ++k;
            if (k > maxMatchLen) {
                maxMatchLen = k;
                bestOffset = i - j;
                if (maxMatchLen == lookahead) break; // can't do better
            }
        }

        if (maxMatchLen >= 3) {
            // emit match token: flag 0x01, offset (2 bytes big-endian), length (1 byte)
            out.push_back(0x01);
            uint16_t offset16 = static_cast<uint16_t>(bestOffset & 0xFFFF);
            out.push_back(static_cast<unsigned char>((offset16 >> 8) & 0xFF));
            out.push_back(static_cast<unsigned char>(offset16 & 0xFF));
            unsigned char lenByte = static_cast<unsigned char>(std::min<size_t>(maxMatchLen, 255));
            out.push_back(lenByte);
            i += maxMatchLen;
        } else {
            // emit literal token: flag 0x00, byte
            out.push_back(0x00);
            out.push_back(data[i]);
            ++i;
        }
    }

    return out;
}

vector<unsigned char> LZ77Compression::decompress(const vector<unsigned char>& data) {
    vector<unsigned char> out;
    size_t i = 0;
    size_t n = data.size();

    while (i < n) {
        unsigned char flag = data[i++];
        if (flag == 0x00) {
            if (i >= n) break; // truncated - break gracefully
            out.push_back(data[i++]);
        } else if (flag == 0x01) {
            if (i + 3 > n) break; // truncated
            uint16_t offset = (static_cast<uint16_t>(data[i]) << 8) | static_cast<uint16_t>(data[i+1]);
            i += 2;
            unsigned char len = data[i++];
            if (offset == 0 || offset > out.size()) {
                // invalid offset — truncate/stop
                break;
            }
            size_t start = out.size() - offset;
            for (size_t k = 0; k < len; ++k) {
                unsigned char c = out[start + k];
                out.push_back(c);
            }
        } else {
            // unknown flag: stop (defensive)
            break;
        }
    }

    return out;
}
