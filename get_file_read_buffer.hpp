#ifndef GET_FILE_READ_BUFFER_HPP
#define GET_FILE_READ_BUFFER_HPP

#include <memory>

namespace {
    std::pair<std::unique_ptr<char[]>, uint64_t>
    getFileReadBuffer(uint64_t size) {
        try {
            auto data = std::make_unique<char[]>(size);
            return std::make_pair(std::move(data), size);
        } catch (const std::bad_alloc &) {
            return std::make_pair(std::unique_ptr<char[]>{}, 0);
        }
    }
}

#endif
