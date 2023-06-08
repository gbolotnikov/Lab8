#include <algorithm>
#include "DublicateHandler.hpp"

void DublicateHandler::fillMap(IFIleSystem& fileSystem) {
    auto& fileList = fileSystem.getFiles(); 
    for (const Path& path : fileList) {
            _files.emplace(path, File());
    }
}

void DublicateHandler::tryOpen(const Path& path, File& file) {
    if (!file.wasOpen) {
        file.wasOpen = true;
        file.size = fs::file_size(path);
        file.stream = std::ifstream(path, std::ios::in | std::ios::binary);
    } else {
        file.stream.open(path, std::ios::in | std::ios::binary);
        file.stream.seekg(file.curPos);
    }
}

void DublicateHandler::close(File& file) {
        file.curPos = file.stream.tellg();
        file.stream.close();
}

bool DublicateHandler::chunkRead(std::ifstream& file, std::string& str) {
    file.read(str.data(), _chunckSize);
    return file.eof();
}

bool DublicateHandler::isEqual(File& file1, File& file2, Hash hashProcessor) {
    if (file1.size != file2.size) {
        return false;
    }
    
    // if (file1.hash.size() > file2.hash.size()) {
    //     std::swap(file1, file2);
    // }

    for (std::size_t i = 0; i < file1.hash.size(); ++i) {
        if (file1.hash[i] != file2.hash[i]) {
            return false;
        }
    }

    for (std::size_t i = file1.hash.size(); i < file2.hash.size(); ++i) {
        std::string str1(_chunckSize, '\0');
        bool endFile = chunkRead(file1.stream, str1);
        auto hashChunck = hashProcessor(str1.data(), _chunckSize);
        file1.hash.push_back(hashChunck);
        if (hashChunck != file2.hash[i]) {
            return false;
        }
        if (endFile) {
            return true;
        }
    }

    while (true) {
        std::string str1(_chunckSize, '\0');
        std::string str2(_chunckSize, '\0');
        bool endFile1 = chunkRead(file1.stream, str1);
        bool endFile2 = chunkRead(file2.stream, str2);
        file1.hash.push_back(hashProcessor(str1.data(), _chunckSize));
        file2.hash.push_back(hashProcessor(str2.data(), _chunckSize));
        if (file2.hash.back() != file1.hash.back()) {
            return false;
        }
        if (endFile1) {
            break;
        }
    }

    return true;
}

void DublicateHandler::findDublicate(Hash hashProcessor) {
    _equalFile.clear();
    for (auto it = _files.begin(); it != _files.end(); ++it) {
        if (_equalFile.count(it->second.hash) > 0) {
            continue;
        }
        for (auto jt = std::next(it); jt != _files.end(); ++jt) {
            tryOpen(it->first, it->second);
            tryOpen(jt->first, jt->second);
            bool equal;
            if (it->second.hash.size() > jt->second.hash.size()) {
                equal = isEqual(jt->second, it->second, hashProcessor);
            } else {
                equal = isEqual(it->second, jt->second, hashProcessor);
            }
            if (equal) {
                auto& path = _equalFile[it->second.hash];
                auto findAndPush = [](std::vector<Path>& path, const Path& newPath){
                    if (std::find(path.begin(), path.end(), newPath) == path.end()) {
                        path.push_back(newPath);
                    }
                };
                findAndPush(path, it->first);
                findAndPush(path, jt->first);
            }
            close(it->second);
            close(jt->second);
        }
    }
}

const std::map<std::vector<size_t>, std::vector<std::filesystem::path>>& DublicateHandler::getDublicate() {
    return _equalFile;
}


