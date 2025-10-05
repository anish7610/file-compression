// huffman.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <iterator>
#include <string>
#include <cstdint>
#include <bitset>
#include <limits>

using namespace std;

struct Node {
    unsigned char ch;   // representative char (for deterministic tie-break)
    uint64_t freq;
    Node* left;
    Node* right;
    Node(unsigned char c, uint64_t f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    Node(Node* l, Node* r)
        : ch(min(l->ch, r->ch)), freq(l->freq + r->freq), left(l), right(r) {}
};

// Deterministic comparator: sort by freq asc, then by ch asc.
// This guarantees stable tree construction when frequencies tie.
struct Compare {
    bool operator()(const Node* a, const Node* b) const {
        if (a->freq == b->freq) return a->ch > b->ch;
        return a->freq > b->freq;
    }
};

void freeTree(Node* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

void buildCodes(Node* root, string &cur, vector<string> &codes) {
    if (!root) return;
    if (!root->left && !root->right) {
        // leaf
        codes[root->ch] = cur.empty() ? "0" : cur; // if only one symbol, give "0"
        return;
    }
    cur.push_back('0'); buildCodes(root->left, cur, codes); cur.pop_back();
    cur.push_back('1'); buildCodes(root->right, cur, codes); cur.pop_back();
}

bool compressFile(const string &inPath, const string &outPath) {
    // read input file
    ifstream in(inPath, ios::binary);
    if (!in) { cerr << "Error: cannot open input file\n"; return false; }
    vector<unsigned char> data((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();

    // frequency table
    unordered_map<unsigned char, uint64_t> freq;
    for (unsigned char c : data) freq[c]++;

    // Edge: empty file
    if (data.empty()) {
        ofstream out(outPath, ios::binary);
        // write empty header
        uint16_t tableSize = 0;
        out.write(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
        uint64_t originalSize = 0;
        out.write(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));
        out.close();
        cout << "Compressed empty file -> " << outPath << "\n";
        return true;
    }

    // build priority queue
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto &p : freq) pq.push(new Node(p.first, p.second));

    while (pq.size() > 1) {
        Node* a = pq.top(); pq.pop();
        Node* b = pq.top(); pq.pop();
        pq.push(new Node(a, b));
    }
    Node* root = pq.top();

    // build codes (vector indexed by byte value)
    vector<string> codes(256);
    string cur;
    buildCodes(root, cur, codes);

    // create bitstring
    string bitstr;
    bitstr.reserve(data.size() * 4); // rough reservation
    for (unsigned char c : data) {
        bitstr += codes[c];
    }

    // padding
    uint8_t extraBits = 0;
    if (bitstr.size() % 8 != 0) {
        extraBits = static_cast<uint8_t>(8 - (bitstr.size() % 8));
        bitstr.append(extraBits, '0');
    } else {
        extraBits = 0;
    }

    // prepare output file and write header:
    // [uint16_t tableSize]
    // for each table entry: [unsigned char ch][uint64_t freq]
    // [uint64_t originalSize]
    // [uint8_t extraBits]
    // [bytes of encoded data]

    ofstream out(outPath, ios::binary);
    if (!out) { cerr << "Error: cannot open output file\n"; freeTree(root); return false; }

    uint16_t tableSize = static_cast<uint16_t>(freq.size());
    out.write(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
    for (auto &p : freq) {
        out.put(static_cast<char>(p.first));
        out.write(reinterpret_cast<char*>(&p.second), sizeof(p.second));
    }

    uint64_t originalSize = data.size();
    out.write(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));

    out.put(static_cast<char>(extraBits));

    // write encoded bytes
    for (size_t i = 0; i < bitstr.size(); i += 8) {
        uint8_t val = 0;
        for (int b = 0; b < 8; ++b) {
            if (bitstr[i + b] == '1') val |= (1u << (7 - b));
        }
        out.put(static_cast<char>(val));
    }

    out.close();
    freeTree(root);
    cout << "Compressed " << inPath << " -> " << outPath << " (orig bytes = " << originalSize << ")\n";
    return true;
}

bool decompressFile(const string &inPath, const string &outPath) {
    ifstream in(inPath, ios::binary);
    if (!in) { cerr << "Error: cannot open compressed file\n"; return false; }

    // read table size
    uint16_t tableSize = 0;
    in.read(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
    unordered_map<unsigned char, uint64_t> freq;
    for (uint16_t i = 0; i < tableSize; ++i) {
        int ch = in.get();
        uint64_t f = 0;
        in.read(reinterpret_cast<char*>(&f), sizeof(f));
        freq[static_cast<unsigned char>(ch)] = f;
    }

    // read original size
    uint64_t originalSize = 0;
    in.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));

    // edge: empty original
    if (originalSize == 0 && tableSize == 0) {
        ofstream out(outPath, ios::binary);
        out.close();
        cout << "Decompressed empty file -> " << outPath << "\n";
        return true;
    }

    // build Huffman tree deterministically from freq
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto &p : freq) pq.push(new Node(p.first, p.second));
    while (pq.size() > 1) {
        Node* a = pq.top(); pq.pop();
        Node* b = pq.top(); pq.pop();
        pq.push(new Node(a, b));
    }
    Node* root = pq.top();

    // read extraBits
    int extraBitsInt = in.get();
    if (extraBitsInt == EOF) { cerr << "Error: truncated compressed file\n"; freeTree(root); return false; }
    uint8_t extraBits = static_cast<uint8_t>(extraBitsInt);

    // read remaining bytes into vector
    vector<unsigned char> encodedBytes((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();

    // build bitstring
    string bitstr;
    bitstr.reserve(encodedBytes.size() * 8ULL);
    for (unsigned char b : encodedBytes) {
        for (int i = 7; i >= 0; --i) bitstr.push_back((b & (1u << i)) ? '1' : '0');
    }
    if (extraBits && bitstr.size() >= extraBits) {
        bitstr.erase(bitstr.end() - extraBits, bitstr.end());
    }

    // decode bits until we've produced originalSize bytes
    ofstream out(outPath, ios::binary);
    if (!out) { cerr << "Error: cannot open output file for decompression\n"; freeTree(root); return false; }

    Node* cur = root;
    uint64_t produced = 0;
    for (size_t i = 0; i < bitstr.size() && produced < originalSize; ++i) {
        cur = (bitstr[i] == '0') ? cur->left : cur->right;
        if (!cur->left && !cur->right) {
            out.put(static_cast<char>(cur->ch));
            produced++;
            cur = root;
        }
    }

    out.close();
    freeTree(root);

    if (produced != originalSize) {
        cerr << "Warning: decompressed size mismatch (expected " << originalSize << ", got " << produced << ")\n";
        return false;
    }

    cout << "Decompressed " << inPath << " -> " << outPath << " (bytes = " << produced << ")\n";
    return true;
}

void printUsage(const char* prog) {
    cout << "Usage:\n"
         << "  " << prog << " -c <input> <output>   # compress\n"
         << "  " << prog << " -d <input> <output>   # decompress\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) { printUsage(argv[0]); return 1; }
    string mode = argv[1];
    string inPath = argv[2];
    string outPath = argv[3];

    if (mode == "-c") {
        if (!compressFile(inPath, outPath)) return 1;
    } else if (mode == "-d") {
        if (!decompressFile(inPath, outPath)) return 1;
    } else {
        printUsage(argv[0]);
        return 1;
    }
    return 0;
}
