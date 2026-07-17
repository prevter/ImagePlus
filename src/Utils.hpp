#pragma once
#include <memory>

namespace imgp::util {
    inline std::unique_ptr<uint8_t[]> make_unique(size_t size) noexcept {
        return std::unique_ptr<uint8_t[]>(new (std::nothrow) uint8_t[size]);
    }
}