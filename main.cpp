#include "format.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <limits>
#include <spdlog/spdlog.h>
#include <span>
#include <cxxopts.hpp>
#include "get_file_read_buffer.hpp"
#include <vector>
#include <string_view>

#define _make_exception_(name)                                                 \
  struct name : public std::runtime_error {                                    \
    name(const std::string& msg) : std::runtime_error(msg){};                  \
  }

_make_exception_(FileDoesNotExist);
_make_exception_(NotAFile);
_make_exception_(FileException);

bool fileByteCompare(const std::filesystem::path& path1,
                     const std::filesystem::path& path2,
                     std::span<char> buffer1, std::span<char> buffer2) {
    if (buffer1.size() >
        static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))
        throw std::logic_error("Could not compare files as buffer1 is larger than "
                               "the maximum input read stream input size.");
    if (buffer2.size() >
        static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))
        throw std::logic_error("Could not compare files as buffer2 is larger than "
                               "the maximum input read stream input size.");
    if (!std::filesystem::exists(path1))
        throw FileDoesNotExist(
                FORMAT_LIB::format("The file \"{}\" could not be found", path1));
    if (!std::filesystem::exists(path2))
        throw FileDoesNotExist(
                FORMAT_LIB::format("The file \"{}\" could not be found", path2));
    if (!std::filesystem::is_regular_file(path1))
        throw NotAFile(FORMAT_LIB::format("The path \"{}\" is not a file", path1));
    if (!std::filesystem::is_regular_file(path2))
        throw NotAFile(FORMAT_LIB::format("The path \"{}\" is not a file", path2));

    std::basic_ifstream<char> inputStream1(path1, std::ios_base::binary);
    if (inputStream1.bad() || !inputStream1.is_open()) {
        throw FileException(FORMAT_LIB::format(
                "There was an error opening \"{}\" for reading", path1));
    }

    std::basic_ifstream<char> inputStream2(path2, std::ios_base::binary);
    if (inputStream2.bad() || !inputStream2.is_open()) {
        throw FileException(FORMAT_LIB::format(
                "There was an error opening \"{}\" for reading", path2));
    }

    auto readSize =
            static_cast<std::streamsize>(std::min(buffer1.size(), buffer2.size()));

    while (!inputStream1.eof() && !inputStream2.eof()) {
        inputStream1.read(buffer1.data(), readSize);
        inputStream2.read(buffer2.data(), readSize);

        if (inputStream1.bad()) {
            throw FileException(
                    FORMAT_LIB::format("There was an error reading \"{}\"", path1));
        }
        if (inputStream2.bad()) {
            throw FileException(
                    FORMAT_LIB::format("There was an error reading \"{}\"", path2));
        }

        uint64_t read1 = static_cast<uint64_t>(inputStream1.gcount());
        uint64_t read2 = static_cast<uint64_t>(inputStream2.gcount());

        if (read1 != read2)
            return false;

        std::span<char> readBuffer1 = {buffer1.data(), read1};
        std::span<char> readBuffer2 = {buffer2.data(), read2};

        if (!std::ranges::equal(readBuffer1, readBuffer2))
            return false;
    }

    return true;
}




std::string_view removePrefix(std::string_view str,
                              const std::string_view prefix) {
    if (str.starts_with(prefix))
        str.remove_prefix(prefix.size());
    return str;
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);
    cxxopts::Options options("File Structure Comparison", "Check that files in one file tree exist and are the same as in another file tree.");

    // check that all the files in 'reference' are in 'base' (but base can have additional files)
    options.add_options()
            ("r,reference", "The root directory or file to act as a reference", cxxopts::value<std::filesystem::path>())
            ("b,base", "The root directory or file to act as a base", cxxopts::value<std::filesystem::path>())
            ("s,size", "The buffer size for the comparison", cxxopts::value<uint64_t>()->default_value("1073741824"));

    auto result = options.parse(argc, argv);

    if (result.count("reference") <= 0 )
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    if (result.count("base") <= 0 )
    {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    auto reference = result["reference"].as<std::filesystem::path>();
    auto base = result["base"].as<std::filesystem::path>();
    auto bufferSize = result["size"].as<uint64_t>();

    auto [dataPointer1, size1] = getFileReadBuffer(bufferSize);
    auto [dataPointer2, size2] = getFileReadBuffer(bufferSize);

    if (size1 != bufferSize || size2 != bufferSize){
        spdlog::error("Could not allocate read buffers!");
        return EXIT_FAILURE;
    }

    std::span buffer1 = {dataPointer1.get(), size1};
    std::span buffer2 = {dataPointer2.get(), size2};

    spdlog::info("Reference path: {}\n Base path: {}", reference, base);


    if (!std::filesystem::exists(reference)){
        spdlog::error("Reference path does not exist!");
        return EXIT_FAILURE;
    }
    if (!std::filesystem::exists(base)){
        spdlog::error("Base path does not exist!");
        return EXIT_FAILURE;
    }

    if (std::filesystem::is_regular_file(reference)){
        spdlog::info("Reference path is a file.");
        if (!std::filesystem::is_regular_file(base)){
            spdlog::error("Base path is not a file, this is an error.");
            return EXIT_FAILURE;
        }

        if(!fileByteCompare(reference, base, buffer1, buffer2)){
            spdlog::info("Files are not equal!");
        }
        else{
            spdlog::info("File are equal.");
        }
    }
    else if (std::filesystem::is_directory(reference)){
        spdlog::info("Reference path is a directory.");
        if (!std::filesystem::is_directory(base)){
            spdlog::error("Base path is not a directory, this is an error.");
            return EXIT_FAILURE;
        }

        std::vector<std::filesystem::path> notExists;
        std::vector<std::filesystem::path> missmatch;

        std::string referencePath = reference.native();
        std::string basePath = base.native();

        for(auto& directoryEntry : std::filesystem::recursive_directory_iterator(reference)){
            std::string currentPath = directoryEntry.path().native();

            spdlog::info("Processing: {}", currentPath);

            auto relativePathView = removePrefix(currentPath, referencePath);
            if(relativePathView.starts_with('/')){
                relativePathView = removePrefix(relativePathView, "/");
            }
            const auto relativePath = std::string{relativePathView};
            const std::filesystem::path newPath = base / relativePath;

            if(!std::filesystem::exists(newPath)){
                notExists.push_back(newPath);
            }
            else if(directoryEntry.is_regular_file()){
                if(!fileByteCompare(directoryEntry.path(), newPath, buffer1, buffer2)){
                    missmatch.push_back(newPath);
                }
            }
        }

        spdlog::info("All items processed");

        for (const auto& path : notExists){
            spdlog::warn("Missing: {}", path.native());
        }
        for (const auto& path : missmatch){
            spdlog::warn("Not Equal: {}", path.native());
        }
    }
    else{
        spdlog::error("Reference path was of unknown type");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
