### File Compression - Yet Another Fun Side Project

## Compile & Run

```
clang++ -std=c++17 -O2 -Wall -Wextra \
    -o compressor main.cpp huffman.cpp lz77.cpp rle.cpp utils.cpp

# Huffman example
./compressor huffman c sample.txt compressed.huff
./compressor huffman d compressed.huff decompressed.txt
diff sample.txt decompressed.txt

# LZ77 example
./compressor lz77 c sample.txt compressed.lz77
./compressor lz77 d compressed.lz77 decompressed.txt
diff sample.txt decompressed.txt

# RLE example
./compressor rle c sample.txt compressed.rle
./compressor rle d compressed.rle decompressed.txt
diff sample.txt decompressed.txt
```

## Using Makefile Targets

```
make              # Compile compressor
make run-huffman  # Huffman compress/decompress test
make run-lz77     # LZ77 compress/decompress test
make run-rle      # RLE compress/decompress test
make run-all      # Run all supported modes
make clean        # Remove binary and generated files
```

## In Python

- python file-compress.py

## Check that:

- compressed.huff is created.
- diff sample.txt decompressed.txt

### Future Improvements

- Improvements for Real Usage
- Store frequency table inside compressed file so decompression can rebuild the tree without original text.
- Handle binary files (rb/wb mode everywhere).
- Use pickle or JSON to serialize metadata.
- Error handling for corrupted files.


### Thought Process

- https://chatgpt.com/share/689a27c6-eac0-8006-8668-bb68237ea752
