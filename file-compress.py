import heapq
import os
from collections import defaultdict, Counter

class HuffmanNode:
    def __init__(self, char, freq):
        self.char = char
        self.freq = freq
        self.left = None
        self.right = None

    # For heapq comparison
    def __lt__(self, other):
        return self.freq < other.freq


def build_huffman_tree(frequency):
    heap = [HuffmanNode(char, freq) for char, freq in frequency.items()]
    heapq.heapify(heap)

    while len(heap) > 1:
        node1 = heapq.heappop(heap)
        node2 = heapq.heappop(heap)
        merged = HuffmanNode(None, node1.freq + node2.freq)
        merged.left = node1
        merged.right = node2
        heapq.heappush(heap, merged)

    return heap[0]

def generate_codes(node, current_code="", codes={}):
    if node is None:
        return

    if node.char is not None:
        codes[node.char] = current_code
        return

    generate_codes(node.left, current_code + "0", codes)
    generate_codes(node.right, current_code + "1", codes)

    return codes

def compress(input_file, output_file):
    # Read input file
    with open(input_file, 'r', encoding='utf-8') as f:
        text = f.read()

    # Frequency table
    frequency = Counter(text)

    # Build tree & codes
    root = build_huffman_tree(frequency)
    codes = generate_codes(root)

    # Encode text
    encoded_text = ''.join(codes[char] for char in text)

    # Padding
    extra_padding = 8 - len(encoded_text) % 8
    encoded_text += "0" * extra_padding

    # Store padding info in 8 bits
    padded_info = "{0:08b}".format(extra_padding)
    encoded_text = padded_info + encoded_text

    # Convert binary string to bytes
    b = bytearray()
    for i in range(0, len(encoded_text), 8):
        byte = encoded_text[i:i+8]
        b.append(int(byte, 2))

    # Save compressed file
    with open(output_file, 'wb') as out:
        out.write(b)

    print(f"File compressed and saved as {output_file}")

def decompress(input_file, output_file, root):
    with open(input_file, 'rb') as f:
        bit_string = ""
        byte = f.read(1)
        while byte:
            byte = ord(byte)
            bits = bin(byte)[2:].rjust(8, '0')
            bit_string += bits
            byte = f.read(1)

    # Remove padding
    padding_info = bit_string[:8]
    extra_padding = int(padding_info, 2)
    bit_string = bit_string[8:]  # remove padding info
    encoded_text = bit_string[:-extra_padding]

    # Decode
    decoded_text = ""
    current_node = root
    for bit in encoded_text:
        current_node = current_node.left if bit == '0' else current_node.right
        if current_node.char is not None:
            decoded_text += current_node.char
            current_node = root

    with open(output_file, 'w', encoding='utf-8') as out:
        out.write(decoded_text)

    print(f"File decompressed and saved as {output_file}")

if __name__ == "__main__":
    compress("sample.txt", "compressed.huff")

    # NOTE: For decompression, you must have the same Huffman tree (root)
    # In practice, you’d serialize the tree or frequency table along with the compressed data.
    frequency = Counter(open("sample.txt", encoding='utf-8').read())
    root = build_huffman_tree(frequency)
    decompress("compressed.huff", "output.txt", root)
