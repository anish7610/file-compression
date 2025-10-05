// huffman.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <bitset>
#include <string>
#include <iterator>
#include <cstdint>

using namespace std;

struct Node {
    unsigned char ch;
    uint32_t freq;
    Node* left;
    Node* right;
    Node(unsigned char c, uint32_t f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r) : ch(0), freq(l->freq + r->freq), left(l), right(r) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) const {
        return a->freq > b->freq;
    }
};

void freeTree(Node* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

void generateCodes(Node* root, const string& prefix, unordered_map<unsigned char, string>& codes) {
    if (!root) return;
    // Leaf node
    if (!root->left && !root->right) {
        // Edge case: if prefix empty (single unique symbol in file), give it "0"
        codes[root->ch] = prefix.empty() ? "0" : prefix;
        return;
    }
    if (root->left)  generateCodes(root->left,  prefix + "0", codes);
    if (root->right) generateCodes(root->right, prefix + "1", codes);
}

void compress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in) {
        cerr << "Error opening input file for reading: " << inputFile << "\n";
        return;
    }

    // Read whole file into vector<unsigned char>
    vector<unsigned char> data((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();

    // Frequency table (uint32_t for portability)
    unordered_map<unsigned char, uint32_t> freq;
    for (unsigned char c : data) freq[c]++;

    // Open output
    ofstream out(outputFile, ios::binary);
    if (!out) {
        cerr << "Error opening output file for writing: " << outputFile << "\n";
        return;
    }

    // If file empty, write zero table and exit
    if (freq.empty()) {
        uint16_t tableSize = 0;
        out.write(reinterpret_cast<const char*>(&tableSize), sizeof(tableSize));
        out.close();
        cout << "Compression complete (empty input): " << outputFile << "\n";
        return;
    }

    // Build Huffman tree
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto &p : freq) {
        pq.push(new Node(p.first, p.second));
    }
    while (pq.size() > 1) {
        Node* l = pq.top(); pq.pop();
        Node* r = pq.top(); pq.pop();
        pq.push(new Node(l, r));
    }
    Node* root = pq.top();

    // Generate codes
    unordered_map<unsigned char, string> codes;
    generateCodes(root, "", codes);

    // Write frequency table:
    // [uint16_t tableSize] then for each entry: [1 byte symbol][uint32_t freq]
    uint16_t tableSize = static_cast<uint16_t>(freq.size());
    out.write(reinterpret_cast<const char*>(&tableSize), sizeof(tableSize));
    for (auto &p : freq) {
        unsigned char symbol = p.first;
        uint32_t f = p.second;
        out.put(static_cast<char>(symbol));
        out.write(reinterpret_cast<const char*>(&f), sizeof(f));
    }

    // Encode data to bitstring
    string encoded;
    encoded.reserve(data.size() * 2); // heuristic reserve
    for (unsigned char c : data) encoded += codes[c];

    // Padding: extraBits in [0..7]
    uint8_t extraBits = static_cast<uint8_t>((8 - (encoded.size() % 8)) % 8);
    if (extraBits) encoded.append(extraBits, '0');

    // Write extraBits as 1 byte (so decompressor knows how many to strip)
    out.put(static_cast<char>(extraBits));

    // Write encoded bits as bytes
    for (size_t i = 0; i < encoded.size(); i += 8) {
        bitset<8> bits(encoded.substr(i, 8));
        unsigned char b = static_cast<unsigned char>(bits.to_ulong());
        out.put(static_cast<char>(b));
    }

    freeTree(root);
    out.close();
    cout << "Compression complete: " << outputFile << "\n";
}

void decompress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in) {
        cerr << "Error opening compressed file for reading: " << inputFile << "\n";
        return;
    }

    // Read table size
    uint16_t tableSize = 0;
    in.read(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
    if (!in) {
        cerr << "Error reading frequency table size\n";
        return;
    }

    // Empty file case
    if (tableSize == 0) {
        // Create empty output
        ofstream outEmpty(outputFile, ios::binary);
        outEmpty.close();
        cout << "Decompression complete (empty input): " << outputFile << "\n";
        return;
    }

    unordered_map<unsigned char, uint32_t> freq;
    for (uint16_t i = 0; i < tableSize; ++i) {
        int ch = in.get();
        if (ch == EOF) {
            cerr << "Unexpected EOF while reading symbol from table\n";
            return;
        }
        unsigned char symbol = static_cast<unsigned char>(ch);
        uint32_t f = 0;
        in.read(reinterpret_cast<char*>(&f), sizeof(f));
        if (!in) {
            cerr << "Unexpected EOF while reading frequency from table\n";
            return;
        }
        freq[symbol] = f;
    }

    // Read extraBits byte
    int eb = in.get();
    if (eb == EOF) {
        cerr << "Unexpected EOF while reading padding info\n";
        return;
    }
    uint8_t extraBits = static_cast<uint8_t>(eb);

    // Read remaining bytes -> build bitstring
    string bitstring;
    // read all remaining bytes
    char buffer;
    while (in.get(buffer)) {
        unsigned char b = static_cast<unsigned char>(buffer);
        bitset<8> bits(b);
        bitstring += bits.to_string();
    }
    in.close();

    // Remove padding bits (if any)
    if (extraBits > 0) {
        if (extraBits > bitstring.size()) {
            cerr << "Corrupt compressed data: padding larger than data\n";
            return;
        }
        bitstring.erase(bitstring.size() - extraBits, extraBits);
    }

    // Rebuild Huffman tree
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto &p : freq) pq.push(new Node(p.first, p.second));
    while (pq.size() > 1) {
        Node* l = pq.top(); pq.pop();
        Node* r = pq.top(); pq.pop();
        pq.push(new Node(l, r));
    }
    Node* root = pq.top();

    // Special case: single unique symbol
    if (!root->left && !root->right) {
        // Write symbol freq times (sum of freq values)
        uint64_t total = 0;
        for (auto &p : freq) total += p.second;
        ofstream out(outputFile, ios::binary);
        for (uint64_t i = 0; i < total; ++i) out.put(static_cast<char>(root->ch));
        out.close();
        freeTree(root);
        cout << "Decompression complete: " << outputFile << "\n";
        return;
    }

    // Decode by traversing tree for each bit
    ofstream out(outputFile, ios::binary);
    Node* current = root;
    for (char bit : bitstring) {
        current = (bit == '0') ? current->left : current->right;
        if (!current->left && !current->right) {
            out.put(static_cast<char>(current->ch));
            current = root;
        }
    }
    out.close();
    freeTree(root);
    cout << "Decompression complete: " << outputFile << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage:\n"
             << "  " << argv[0] << " -c input.bin output.huff   (compress)\n"
             << "  " << argv[0] << " -d input.huff output.bin   (decompress)\n";
        return 1;
    }

    string mode = argv[1];
    if (mode == "-c") {
        compress(argv[2], argv[3]);
    } else if (mode == "-d") {
        decompress(argv[2], argv[3]);
    } else {
        cerr << "Invalid mode. Use -c to compress or -d to decompress.\n";
        return 1;
    }

    return 0;
}
