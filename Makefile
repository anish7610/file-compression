CXX := g++
CXXFLAGS := -std=c++17 -Wall -O2 -Iinclude
SRC_DIR := src
OBJ_DIR := obj
TARGET := compressor

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) *.huff *.rle *.lz77 decompressed_*

run-huffman:
	./$(TARGET) huffman c sample.txt compressed.huff
	./$(TARGET) huffman d compressed.huff decompressed_huff.txt

run-rle:
	./$(TARGET) rle c sample.txt compressed.rle
	./$(TARGET) rle d compressed.rle decompressed_rle.txt

run-lz77:
	./$(TARGET) lz77 c sample.txt compressed.lz77
	./$(TARGET) lz77 d compressed.lz77 decompressed_lz77.txt

# Run all
run-all: run-huffman run-lz77 run-rle

.PHONY: all clean run-huffman run-rle run-lz77
