#ifndef IFILE_SYSTEM_HPP
#define IFILE_SYSTEM_HPP

#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class IFIleSystem {
public:
    virtual const std::vector<fs::path>& getFiles() = 0;
    virtual ~IFIleSystem() = default;
};

#endif