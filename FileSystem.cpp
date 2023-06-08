#include "FileSystem.hpp"
#include <algorithm>

namespace fs = std::filesystem;

bool FileSystem::alreadyExist(const fs::path& path) {
    auto iter = std::find(_directories.begin(), _directories.end(), path);
    return iter != _directories.end();
}

void FileSystem::addDirectories(const fs::path& path) {
    fs::path normalPath = path.lexically_normal();
    if (alreadyExist(normalPath)) {
        return;
    }
    if (_recursive) {
        for(auto rootIt = _directories.cbegin(); rootIt != _directories.cend();) {
            auto [rootEnd, pathEnd] = std::mismatch(rootIt->begin(), rootIt->end(), normalPath.begin(), normalPath.end());
            if (rootEnd == rootIt->end()) {
                return;
            }
            if (pathEnd == normalPath.end()) {
                _directories.erase(rootIt);
            } else {
                ++rootIt; // иначе потеряем элемент
            }
        }
    }
    _directories.push_back(std::move(normalPath));
}

void FileSystem::addExcludeDir(const fs::path& path) {
    fs::path normalPath = path.lexically_normal();
    _excludeDir.push_back(std::move(normalPath));
}

void FileSystem::excludeDirectories(const fs::path& path) {
    fs::path normalPath = path.lexically_normal();
    auto removeDir = std::find(_directories.begin(), _directories.end(), normalPath);
    if (removeDir != _directories.end()) {
        _directories.erase(removeDir);
    }
}

void FileSystem::addMasksFile(const std::regex& mask) {
    _masks.push_back(std::move(mask));
}

bool FileSystem::needExclude(const fs::path& path) {
    return (std::find(_excludeDir.begin(), _excludeDir.end(), path) != _excludeDir.end());
}

void FileSystem::addFiles(fs::directory_iterator& dirIt) {
    for (const fs::directory_entry& file : dirIt) {
        if (file.is_regular_file() && matchCondition(file)) {
            _fileList.push_back(file);
        } else if (_recursive && file.is_directory() && !needExclude(file)) {
            fs::directory_iterator dirIt(file);
            addFiles(dirIt);
        }
    }
}       

const std::vector<fs::path>& FileSystem::getFiles() {
    for (const auto& pathDir : _directories) {
        fs::directory_iterator dirIt(pathDir);
        addFiles(dirIt);
    }
    return _fileList;
}

bool FileSystem::matchCondition(const fs::path& path) {
    if (_minFileSize != 0)  {
        if (fs::file_size(path) < _minFileSize) {
            return false;
        }
    }

    if (_masks.empty()) {
        return true;
    }

    for (const std::regex& mask : _masks) {
        auto pathDeb = path.generic_string();
        if (std::regex_match(pathDeb, mask)) {
            return true;
        }   
    }
    return false;
}
