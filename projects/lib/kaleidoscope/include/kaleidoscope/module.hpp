#pragma once

#include <kaleidoscope/ast.hpp>
#include <kaleidoscope/type_registry.hpp>

#include <map>
#include <unordered_map>
#include <variant>
#include <string>
#include <list>
#include <functional>
#include <any>
#include <optional>
#include <stdexcept>
#include <format>

#include <numeric>
#include <ranges>
#include <algorithm>

#include <iostream>

namespace kaleidoscope
{
/**
 * @brief Container for kaleidoscope language symbols.
 *
 * Can contain: types, functions, data objects.
 */
struct Module
{
  public: // types

    using Type_Ref   = std::reference_wrapper<const type::Info>;
    using Module_Ref = std::reference_wrapper<const Module>;
    using precedence = std::unordered_map<std::string, type::Operator_Properties>;

    enum class Visibility
    {
        Private,
        Public
    };

    struct Version
    {
        unsigned                   major = 0;
        unsigned                   minor = 0;
        unsigned                   patch = 0;
        unsigned                   tweak = 0;
        std::optional<std::string> details;

        operator std::string(void) const; // fmt({}.{}.{}, major, minor, patch) + (details) ? ("-" + *details) : "";
    };

    struct Callable
    {
        std::string name;
        Type_Ref    type;
    };

    struct Symbol;
    struct Symbol_Key;

  public: // constructors

    Module(std::string name, Version version);

  public: // rule of 5

    Module(const Module &)            = default;
    Module(Module &&)                 = default;
    Module &operator=(const Module &) = default;
    Module &operator=(Module &&)      = default;
    ~Module(void)                     = default;

  public: // symbol transfer

    void link_static(const Module &m);
    void link_shared(const Module &m);

  public:

    type::Info *
    register_type(std::string name, std::unique_ptr<type::Info> info);

    template<traits::Type_Basic_Scalar T>
    type::Info *
    register_type(std::string name);

    Symbol *
    declare_variable(std::string name, type::Info *type, std::unique_ptr<ast::Node> definition);

    Symbol *
    declare_function(std::string name, type::Info *type, std::unique_ptr<ast::Node> definition);

  protected:

    std::string name_;
    Version     version_;

    std::map<Symbol_Key, Symbol> symbols_;
    type::Registry               type_info_;

    std::list<Module_Ref> linked_;
};

struct Module::Symbol
{
};

struct Module::Symbol_Key
{
    size_t      idx; /**< declaration order position from '0' */
    Visibility  visibility;
    std::string name;
    Type_Ref    type;
};

} // namespace kaleidoscope

namespace kaleidoscope
{
Module::Version::
operator std::string(void) const
{
    using namespace std::string_literals;

    return std::format("{}.{}.{}{}", major, minor, tweak, patch, (details) ? ("-"s + *details) : (""s));
}

} // namespace kaleidoscope

namespace kaleidoscope
{
inline Module::Module(std::string name, Version version)
    : name_(name)
    , version_(version)
{
}

inline void
Module::link_static(const Module &m)
{
    // TODO: implement
}

inline void
Module::link_shared(const Module &m)
{
    linked_.push_front(std::ref(m));
}

inline type::Info *
Module::register_type(std::string name, std::unique_ptr<type::Info> info)
{
    return type_info_.emplace(std::move(info));
}

template<traits::Type_Basic_Scalar T>
inline type::Info *
Module::register_type(std::string name)
{
    return type_info_.register_type<T>(name);
}
} // namespace kaleidoscope
