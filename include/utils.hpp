#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

std::vector<unsigned char> readFile(const std::string& filename);
void writeFile(const std::string& filename, const std::vector<unsigned char>& data);

#endif
