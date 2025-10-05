# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -O2

# Target executable
TARGET := huffman

# Source files
SRC := huffman.cpp

# Default build
all: $(TARGET)

# Compile
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Run compression example
run: $(TARGET)
	./$(TARGET) -c sample.txt compressed.huff
	./$(TARGET) -d compressed.huff output.txt

# Clean up
clean:
	rm -f $(TARGET) *.o compressed.huff output.txt

.PHONY: all run clean
