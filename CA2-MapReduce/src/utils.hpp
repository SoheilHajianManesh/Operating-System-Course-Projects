#ifndef UTILS_HPP
#define UTILS_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "logger.hpp"

namespace fs= std::filesystem;

int getDirFolders(std::string path, std::vector<fs::path>& folders,Logger& log) {
    if (fs::exists(path) && fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                folders.push_back(entry.path());
            }
        }
    }
    else {
        log.logError("Wrong path: " + path);
        return EXIT_FAILURE;
    }
    log.logInfo("Folders founded");
    return 1;
}
int getDirFiles(std::string path, std::vector<fs::path>& folders,Logger& log) {
    if (fs::exists(path) && fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
                folders.push_back(entry.path());
        }
    }
    else {
        log.logError("Wrong path: " + path);
        return EXIT_FAILURE;
    }
    log.logInfo("Folders founded");
    return 1;
}

#endif