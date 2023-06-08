#include <boost/program_options.hpp>

#include <iostream>
#include <filesystem>
#include <regex>
#include "FileSystem.hpp"
#include "DublicateHandler.hpp"
#include "Crc16.hpp"
#include "Crc32.hpp"

namespace fs = std::filesystem;
namespace po = boost::program_options;

void printPath(const std::map<std::vector<size_t>, std::vector<fs::path>>& fileMap) {
    for (auto it = fileMap.begin(); it != fileMap.end(); ++it) {
        for (const auto& path: it->second) {
            std::cout << path << '\n';
        }
        std::cout << '\n';
    }
}

int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        bool recursive = false;
        desc.add_options()
            ("help",                                                                            "Print this message")
            ("find_dir",       po::value<std::vector<std::string>>()->required()->multitoken(), "Directories")
            ("exclude_dir",    po::value<std::vector<std::string>>()->multitoken(),             "Exclude direcroties")
            ("recursive",      po::bool_switch(&recursive),                                     "Scan level true - recursive, false - recursive")
            ("min_file_size",  po::value<std::size_t>()->required()->default_value(1),          "Minimum file size to checked")
            ("mask_file",      po::value<std::vector<std::string>>()->multitoken(),             "Masks for files in search")
            ("block_size",     po::value<std::size_t>()->required()->default_value(4096),       "Block size for reading files (at least 1)")
            ("crc",            po::value<std::string>()->required()->default_value("crc32"),    "Name of hashing algorithm, one of 'crc32', 'crc16'");

        po::variables_map vm;
        try {
            po::store(parse_command_line(argc, argv, desc), vm);
            if (vm.count("help") != 0) {
                std::cout << desc << "\n";
                return 0;
            }
            po::notify(vm);
        }
        catch (const po::error& error) {
            std::cerr << "Error while parsing command-line arguments: "
                    << error.what() << "\nPlease use --help to see help message\n";
            return 1;
        }

        std::vector<std::string> rootDirs = vm["find_dir"].as<std::vector<std::string>>();
        std::vector<std::string> excludeDirs{};
        if (vm.count("exclude_dir") != 0) {
            excludeDirs = vm["exclude_dir"].as<std::vector<std::string>>();
        }
        auto minFileSize = vm["min_file_size"].as<std::size_t>();
        std::vector<std::string> masks{};
        if (vm.count("mask_file") != 0) {
            masks = vm["mask_file"].as<std::vector<std::string>>();
        }
        auto blockSize = vm["block_size"].as<std::size_t>();
        if (blockSize == 0) {
            std::cerr << "Block size for reading files (at least 1)\n";
            return 1;
        }
        std::string crc = vm["crc"].as<std::string>();
        
        FileSystem fileSystem(recursive, minFileSize);     
        for (const auto& rootDir: rootDirs) {
            fileSystem.addDirectories(fs::path(std::move(rootDir)));
        }
        for (const auto& excludeDir: excludeDirs) {
            fileSystem.addExcludeDir(fs::path(std::move(excludeDir)));
        }
        for (const auto& mask: masks) {
            fileSystem.addMasksFile(std::regex(std::move(mask), std::regex_constants::icase));
        }

        DublicateHandler dublicateHandler(fileSystem, blockSize);
        if (crc == "crc32") {
            dublicateHandler.findDublicate(crc32);
            printPath(dublicateHandler.getDublicate());
        } else if (crc == "crc16") {
            dublicateHandler.findDublicate(crc16);
            printPath(dublicateHandler.getDublicate());
        } else {
            throw std::invalid_argument(std::string("Unknow hash algorithm ") + crc);
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

