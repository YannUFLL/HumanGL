#include <fstream>
#include <sstream>
#include <string>

    std::string loadFile(const char* path) {
        std::ifstream file(path);
        std::stringstream buf;
        buf << file.rdbuf();
        return buf.str();
    }
