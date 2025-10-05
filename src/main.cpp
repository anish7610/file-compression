#include <iostream>
#include <string>
#include <memory>
#include "utils.hpp"
#include "compression.hpp"
#include "huffman.hpp"
#include "rle.hpp"
#include "lz77.hpp"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <algo:huffman|rle|lz77> <mode:c|d> <input> <output>\n";
        return 1;
    }
    std::string algo = argv[1];
    std::string mode = argv[2];
    std::string inPath = argv[3];
    std::string outPath = argv[4];

    std::vector<unsigned char> input = readFile(inPath);
    std::vector<unsigned char> output;

    std::unique_ptr<Compression> compressor;

    if (algo == "huffman") {
        compressor = std::make_unique<HuffmanCompression>();
    } else if (algo == "rle") {
        compressor = std::make_unique<RLECompression>();
    } else if (algo == "lz77") {
        // default window 4096, lookahead 18 - tune as needed
        compressor = std::make_unique<LZ77Compression>(4096, 18);
    } else {
        std::cerr << "Unknown algorithm: " << algo << "\n";
        return 1;
    }

    if (mode == "c") {
        output = compressor->compress(input);
    } else if (mode == "d") {
        output = compressor->decompress(input);
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }

    writeFile(outPath, output);
    std::cout << "Finished.\n";
    return 0;
}
