### File Compression - Yet Another Fun Side Project

## Compile & Run

- clang++ huffman.cpp -o huffman --std=c++11
- ./huffman -c sample.txt compressed.huff   # Compress
- ./huffman -d compressed.huff decompressed.txt   # Decompress

**Note**: C++ version doesn't work. Mismatched output.

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

- https://chatgpt.com/share/689a2038-48f4-8006-81f3-178e22089e89


