#include "strops.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace strops {

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(str);
    std::string token;

    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
} // namespace strops

