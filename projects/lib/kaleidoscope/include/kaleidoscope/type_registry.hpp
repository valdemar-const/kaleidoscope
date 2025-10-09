#pragma once

#include <string>
#include <string_view>
#include <typeindex>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <any>

namespace kaleidoscope
{

namespace traits
{
    template<typename T>
    concept Type_Basic_Scalar = std::is_fundamental_v<T>
                             && std::is_scalar_v<T>
                             && !std::is_pointer_v<T>
                             && !std::is_array_v<T>;
} // namespace traits

struct Type_Registry
{
    using Default_Constructor = std::function<std::any(void)>;

    struct Type_Info
    {
        std::string         name;
        std::type_index     id;
        Default_Constructor ctor_default;
    };

    template<traits::Type_Basic_Scalar T>
    Type_Registry &
    register_type(std::string_view type_name)
    {
        using Type = std::decay_t<T>;
        auto info  = Type_Info {
                 .name         = type_name,
                 .id           = typeid(Type),
                 .ctor_default = [](void) -> std::any
                {
                    return Type {};
                }
        };

        storage_.emplace(std::make_pair(info.id, std::move(info)));

        return *this;
    }

  protected:

    std::unordered_map<std::type_index, Type_Info>                       storage_;
    std::unordered_map<std::string, std::type_index>                     type_by_name_;
    std::unordered_map<std::type_index, std::unordered_set<std::string>> typedefs_;
};

}; // namespace kaleidoscope
