#ifndef FSC_FORMAT_HPP
#define FSC_FORMAT_HPP


#include <fmt/format.h>
#include <filesystem>


#define FORMAT_LIB fmt


template <typename CharT>
struct fmt::formatter<std::filesystem::path, CharT>
    : public fmt::formatter<std::string, CharT> {
    template <typename FormatContext>
    auto format(const std::filesystem::path& path, FormatContext& fc) {
        return fmt::formatter<std::string, CharT>::format(path.string(), fc);
    };
};



#endif