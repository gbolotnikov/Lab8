#ifndef DUBLICATE_HANDLER_HPP
#define DUBLICATE_HANDLER_HPP

#include <map>
#include <vector>
#include <fstream>
#include <filesystem>
#include "IFileSystem.hpp"

class DublicateHandler {
private:
    using Path = std::filesystem::path;
    using Hash = size_t(*)(void* data, std::size_t length);    
public:
    

    DublicateHandler(IFIleSystem& fileSystem, size_t chunckSize):
        _fileSystem(fileSystem),
        _chunckSize(chunckSize) 
        {
            fillMap(fileSystem);
        }
    void findDublicate(Hash hashProcessor);
    const std::map<std::vector<size_t>, std::vector<std::filesystem::path>>& getDublicate();

private:

    struct File {
        size_t size;
        std::ifstream stream;
        bool wasOpen {false};
        std::vector<size_t> hash;
        std::ifstream::pos_type curPos;
    };

    void tryOpen(const Path& path, File& file);
    void fillMap(IFIleSystem& fileSystem);
    bool isEqual(File& file1, File& file2, Hash hashProcessor);
    bool chunkRead(std::ifstream& file, std::string& str);
    void close(File& file);

    IFIleSystem& _fileSystem;
    std::map<Path, File> _files;
    size_t _chunckSize;
    std::map<std::vector<size_t>, std::vector<Path>> _equalFile;
};

#endif