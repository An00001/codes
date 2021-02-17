#include <array>
#include <bit>
#include <concepts>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ranges>

constexpr int N_INFO_BITS = 4;

using Codeword = std::byte;

constexpr auto distance(const Codeword rhs, const Codeword lhs)
{
    return std::popcount(static_cast<std::uint8_t>(lhs ^ rhs));
}

static constexpr std::uint8_t get_info(const Codeword code)
{
    return std::to_integer<std::uint8_t>(code & std::byte{0xf});
}

template <uint8_t N>
static constexpr Codeword encode()
{
    /*
     * Generator Matrix
     * 1 0 0 0 1 1 0 1
     * 0 1 0 0 1 0 1 1
     * 0 0 1 0 0 1 1 1
     * 0 0 0 1 0 0 1 1
     *
     * Corresponding Parity Equations:
     * b4 = b0 + b1
     * b5 = b0 + b2
     * b6 = b1 + b2 + b3
     * b7 = b0 + b1 + b2 + b3
     *
     * where b0..b3 are information bits.
     */
    static_assert(std::bit_width(N) <= N_INFO_BITS,
            "encode requires 4-bit integral values");

    auto word = static_cast<Codeword>(N);

    const auto b0 = std::byte{0x1};
    const auto b1 = std::byte{0x2};
    const auto b2 = std::byte{0x3};
    const auto b3 = std::byte{0x4};

    word |= ((word & b0) ^ (word & b1)) << 5;
    word |= ((word & b0) ^ (word & b2)) << 6;
    word |= ((word & b1) ^ (word & b2) ^ (word & b3)) << 7;
    word |= ((word & b0) ^ (word & b1) ^ (word & b2) ^ (word & b3)) << 8;
    return word;
}

template <std::unsigned_integral T, T... info, typename C>
static constexpr void create_codewords(
        C& codes,
        std::integer_sequence<T, info...> seq)
{
    ((codes[info] = encode<info>()),...);
}

template <typename T>
static constexpr std::uint8_t decode(const T& codes, const Codeword code)
{
    /*
     * Maximum Liklihood Decoding.
     * Compute the coset codes + code then find minimum distance between code
     * and an element within the coset, this is the most likely candidate.
     */
    auto result = std::min_element(codes.cbegin(), codes.cend(),
            [=](auto x, auto y) {
                return distance(x, code) < distance(y, code);
            }
    );
    return get_info(*result);
}

int main()
{
    static constexpr auto codes = [] {
        std::array<Codeword, 1 << N_INFO_BITS > codes{};
        create_codewords(
            codes,
            std::make_integer_sequence<std::uint8_t, 1 << N_INFO_BITS>{}
        );
        return codes;
    }();

    /* Attempt to decode 0xff into adjacent codeword. */
    auto data = decode(codes, Codeword{0xff});
    std::cout << static_cast<int>(data) << '\n';
}
