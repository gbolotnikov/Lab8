#ifndef FILE_SYSTEM_HPP
#define FILE_SYSTEM_HPP

#include <list>
#include <vector>
#include <regex>
#include <filesystem>
#include "IFileSystem.hpp"



class FileSystem: public IFIleSystem {    
public:
    FileSystem(bool recursive, size_t minFileSize): 
        _recursive(recursive),
        _minFileSize(minFileSize)
    {

    }

    void addExcludeDir(const fs::path& path);
    void addDirectories(const fs::path& path);
    void excludeDirectories(const fs::path& path);
    void addMasksFile(const std::regex& masks);
    const std::vector<fs::path>& getFiles() override;

private:
    bool alreadyExist(const fs::path& path);
    void addFiles(fs::directory_iterator& dirIt);
    bool matchCondition(const fs::path& path);
    bool needExclude(const fs::path& path);

    bool _recursive; 
    size_t _minFileSize;
    std::vector<fs::path> _fileList;
    std::vector<fs::path> _excludeDir;
    std::list<fs::path> _directories;    
    std::vector<std::regex> _masks;

};

#endif