#include "huffman.hpp"
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

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

static void freeTree(Node* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

static void buildCodes(Node* root, string &cur, vector<string> &codes) {
    if (!root) return;
    if (!root->left && !root->right) {
        // leaf
        codes[root->ch] = cur.empty() ? "0" : cur; // if only one symbol, give "0"
        return;
    }
    cur.push_back('0'); buildCodes(root->left, cur, codes); cur.pop_back();
    cur.push_back('1'); buildCodes(root->right, cur, codes); cur.pop_back();
}

// compress: returns vector containing the full custom-format bytes
vector<unsigned char> HuffmanCompression::compress(const vector<unsigned char>& data) {
    vector<unsigned char> out;

    // frequency table
    unordered_map<unsigned char, uint64_t> freq;
    for (unsigned char c : data) freq[c]++;

    // Edge: empty file -> write header with tableSize=0 and originalSize=0
    if (data.empty()) {
        // tableSize (uint16_t) = 0
        uint16_t tableSize = 0;
        out.insert(out.end(), reinterpret_cast<unsigned char*>(&tableSize),
                   reinterpret_cast<unsigned char*>(&tableSize) + sizeof(tableSize));
        uint64_t originalSize = 0;
        out.insert(out.end(), reinterpret_cast<unsigned char*>(&originalSize),
                   reinterpret_cast<unsigned char*>(&originalSize) + sizeof(originalSize));
        return out;
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

    // WRITE HEADER into out vector in same order as original program:
    // [uint16_t tableSize]
    // for each table entry: [unsigned char ch][uint64_t freq]
    // [uint64_t originalSize]
    // [uint8_t extraBits]
    // [bytes of encoded data]

    uint16_t tableSize = static_cast<uint16_t>(freq.size());
    // append tableSize bytes (little-endian as in original code writing raw bytes)
    {
        unsigned char *p = reinterpret_cast<unsigned char*>(&tableSize);
        out.insert(out.end(), p, p + sizeof(tableSize));
    }

    // for each freq entry, append ch then freq (uint64_t)
    for (auto &p : freq) {
        out.push_back(static_cast<unsigned char>(p.first));
        unsigned char *fp = reinterpret_cast<unsigned char*>(&p.second);
        out.insert(out.end(), fp, fp + sizeof(p.second));
    }

    // original size
    uint64_t originalSize = data.size();
    {
        unsigned char *p = reinterpret_cast<unsigned char*>(&originalSize);
        out.insert(out.end(), p, p + sizeof(originalSize));
    }

    // extraBits
    out.push_back(static_cast<unsigned char>(extraBits));

    // encoded bytes: pack 8 bits -> byte (MSB-first ordering to match original)
    for (size_t i = 0; i < bitstr.size(); i += 8) {
        uint8_t val = 0;
        for (int b = 0; b < 8; ++b) {
            if (bitstr[i + b] == '1') val |= (1u << (7 - b));
        }
        out.push_back(static_cast<unsigned char>(val));
    }

    freeTree(root);
    return out;
}

// decompress: expects data vector formatted exactly as above; returns original bytes
vector<unsigned char> HuffmanCompression::decompress(const vector<unsigned char>& data) {
    vector<unsigned char> out;

    size_t idx = 0;
    if (data.size() < sizeof(uint16_t)) return out; // truncated

    // read tableSize
    uint16_t tableSize = 0;
    {
        // copy bytes into tableSize
        memcpy(&tableSize, data.data() + idx, sizeof(tableSize));
        idx += sizeof(tableSize);
    }

    unordered_map<unsigned char, uint64_t> freq;
    // read table entries
    for (uint16_t i = 0; i < tableSize; ++i) {
        if (idx >= data.size()) return out; // truncated
        unsigned char ch = data[idx++];
        if (idx + sizeof(uint64_t) > data.size()) return out; // truncated
        uint64_t f = 0;
        memcpy(&f, data.data() + idx, sizeof(uint64_t));
        idx += sizeof(uint64_t);
        freq[ch] = f;
    }

    // read original size
    if (idx + sizeof(uint64_t) > data.size()) return out;
    uint64_t originalSize = 0;
    memcpy(&originalSize, data.data() + idx, sizeof(uint64_t));
    idx += sizeof(uint64_t);

    // edge: empty original
    if (originalSize == 0 && tableSize == 0) {
        return out; // empty vector
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
    if (idx >= data.size()) { freeTree(root); return out; } // truncated
    uint8_t extraBits = data[idx++];
    // remaining bytes are encoded data
    vector<unsigned char> encodedBytes;
    if (idx < data.size()) {
        encodedBytes.insert(encodedBytes.end(), data.begin() + idx, data.end());
    }

    // build bitstring (MSB-first order)
    string bitstr;
    bitstr.reserve(encodedBytes.size() * 8ULL);
    for (unsigned char b : encodedBytes) {
        for (int i = 7; i >= 0; --i) bitstr.push_back((b & (1u << i)) ? '1' : '0');
    }
    if (extraBits && bitstr.size() >= extraBits) {
        bitstr.erase(bitstr.end() - extraBits, bitstr.end());
    }

    // decode bits until we've produced originalSize bytes
    Node* cur = root;
    uint64_t produced = 0;
    for (size_t i = 0; i < bitstr.size() && produced < originalSize; ++i) {
        cur = (bitstr[i] == '0') ? cur->left : cur->right;
        if (!cur->left && !cur->right) {
            out.push_back(static_cast<unsigned char>(cur->ch));
            produced++;
            cur = root;
        }
    }

    freeTree(root);

    // produced should equal originalSize if data well-formed
    return out;
}
