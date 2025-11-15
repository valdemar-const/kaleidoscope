#pragma once

#include <concepts>
#include <type_traits>

#include <boost/callable_traits.hpp>

namespace kaleidoscope
{

namespace traits
{
    template<typename T>
    concept Type_Basic_Scalar = (std::is_fundamental_v<std::remove_cvref_t<T>> && std::is_scalar_v<std::remove_cvref_t<T>>)
                             && !std::is_pointer_v<std::remove_cvref_t<T>>
                             && !std::is_array_v<std::remove_cvref_t<T>>;

    template<typename T>
    concept Type_Object_Value_Semantic = std::regular<T>
                                      && std::movable<T>
                                      && !std::same_as<std::any, std::remove_cvref_t<T>>;

    template<typename T>
    concept Type_Callable =
            std::is_function_v<T> || std::is_invocable_v<T>
            || requires(T t) { &std::decay_t<T>::operator(); };
} // namespace traits
} // namespace kaleidoscope
