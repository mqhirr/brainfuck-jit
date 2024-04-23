#ifndef UTILS_H
#define UTILS_H

#include <array>
#include <cstdint>
#include <type_traits>

namespace BFJit {
    template <typename T, std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
    constexpr auto ConvertU8(T v) -> std::array<uint8_t, sizeof(T)> {
        std::array<uint8_t, sizeof(T)> result;

        for (std::size_t i = 0; i < sizeof(T); ++i) {
            result[i] = static_cast<uint8_t>((v >> (i * 8)) & 0xff);
        }

        return result;
    }
}

#endif // UTILS_H