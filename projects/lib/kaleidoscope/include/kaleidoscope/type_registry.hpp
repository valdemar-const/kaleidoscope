#pragma once

#include <kaleidoscope/module.hpp>
#include <kaleidoscope/type_info.hpp>

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

struct Type_Registry
{
    using Default_Constructor = std::function<std::any(void)>;
    using Default_Destructor  = std::function<std::any(void)>;
    using Type_Info           = type::Info;

    Type_Registry(Module &toplevel)
        : global_(toplevel)
    {
    }

    Type_Registry(Type_Registry &&)            = default;
    Type_Registry &operator=(Type_Registry &&) = default;

    template<traits::Type_Basic_Scalar T>
    Type_Registry &
    register_type(std::string_view type_name)
    {
        using Type            = std::decay_t<T>;
        std::type_index index = typeid(Type);
        auto            info  = type::make_info<Type>(std::string {type_name});

        auto [it, is_emplaced] = storage_.emplace(std::make_pair(index, std::move(info)));

        if (is_emplaced)
        {
            type_by_name_.emplace(std::make_pair((*it).second.name(), index));
        }

        return *this;
    }

  protected:

    std::reference_wrapper<Module>                                       global_;
    std::unordered_map<std::type_index, Type_Info>                       storage_;
    std::unordered_map<std::string_view, std::type_index>                type_by_name_;
    std::unordered_map<std::type_index, std::unordered_set<std::string>> typedefs_;
};

}; // namespace kaleidoscope
