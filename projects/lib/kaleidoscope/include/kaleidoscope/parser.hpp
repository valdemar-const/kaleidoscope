#pragma once

#include <kaleidoscope/ast.hpp>

namespace kaleidoscope::concepts
{

template<typename T>
inline constexpr bool is_safe_trivially_copyable_v =
        std::is_trivially_copyable_v<T>
        && !std::is_polymorphic_v<T> // Блокирует virtual-методы
        && !std::is_pointer_v<T>;    // Блокирует сырые указатели

template<typename Iterator>
concept TriviallyCopyableIterator =
        std::contiguous_iterator<Iterator>
        && is_safe_trivially_copyable_v<
                std::remove_cvref_t<
                        typename std::iterator_traits<Iterator>::value_type>>;
} // namespace kaleidoscope::concepts

namespace kaleidoscope
{

struct Parser
{
    template<concepts::TriviallyCopyableIterator T>
    static Ast parse(T begin, T end);
};

} // namespace kaleidoscope
