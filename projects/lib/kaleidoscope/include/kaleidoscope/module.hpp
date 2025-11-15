#pragma once

#include <kaleidoscope/ast.hpp>
#include <kaleidoscope/type_traits.hpp>
#include <compiler/demangle.hpp>

#include <boost/container_hash/hash.hpp>

#include <map>
#include <unordered_map>
#include <variant>
#include <string>
#include <list>
#include <functional>
#include <any>
#include <optional>
#include <stdexcept>

#include <numeric>
#include <ranges>
#include <algorithm>

#include <iostream>

namespace kaleidoscope
{
class Module;
} // namespace kaleidoscope

namespace std
{
template<>
struct hash<::kaleidoscope::Module::Symbol_Key>;
} // namespace std

namespace kaleidoscope
{

struct Module
{
    struct Symbol; // forward decl

    enum class Symbol_Kind
    {
        Type,
        Func,
        Infix,
        Prefix,
        Postfix,
        DataObject
    };

    struct Proc_Signature
    {
        using Args = std::vector<std::type_index>;

        std::type_index result;
        Args            args;

        auto
        as_tuple() const
        {
            return std::tie(result, args);
        }

        bool
        operator==(const Proc_Signature &other) const
        {
            return as_tuple() == other.as_tuple();
        }

        bool
        operator<(const Proc_Signature &other) const
        {
            return as_tuple() < other.as_tuple();
        }

        size_t
        fingerprint(void) const
        {
            size_t seed;
            boost::hash_combine(seed, result);

            for (const auto &arg : args)
            {
                boost::hash_combine(seed, arg);
            }
            return seed;
        }

        template<traits::Type_Callable T>
        static const Proc_Signature &
        from(void)
        {
            using Callable       = std::decay_t<T>;
            using Args           = boost::callable_traits::args_t<Callable>;
            using Result         = boost::callable_traits::return_type_t<Callable>;
            constexpr auto arity = std::tuple_size_v<Args>;

            static constexpr auto get_args_signatures = []<typename Tuple>(void) -> const std::vector<std::type_index> &
            {
                static const auto result = []<size_t... I>(std::index_sequence<I...>) -> std::vector<std::type_index>
                {
                    std::vector<std::type_index> args_type_signatures;
                    (args_type_signatures.emplace_back(typeid(std::tuple_element_t<I, Tuple>)), ...);
                    return args_type_signatures;
                }(std::make_index_sequence<std::tuple_size_v<Tuple>> {});
                return result;
            };

            static const Proc_Signature result {
                typeid(Result),
                get_args_signatures.template operator()<Args>()
            };
            return result;
        }
    };

    struct Operator_Properties
    {
        using Precedence = size_t;

        enum class Associativity : uint8_t
        {
            Left,
            Right
        };

        enum class Kind : uint8_t
        {
            Postfix,
            Prefix,
            Infix
        };

        Kind          kind          = Kind::Infix;
        Associativity associativity = Associativity::Left;
        Precedence    precedence    = 0; /**< lesser is higher */
    };

    struct Symbol_Key
    {
        std::string                   name;
        Symbol_Kind                   kind;
        std::optional<Proc_Signature> overload;

        auto
        as_tuple() const
        {
            return std::tie(name, kind, overload);
        }

        bool
        operator==(const Symbol_Key &other) const
        {
            return as_tuple() == other.as_tuple();
        }

        bool
        operator<(const Symbol_Key &other) const
        {
            return as_tuple() < other.as_tuple();
        }
    };

    using Symbol_Name          = std::string;
    using Symbol_Idx           = size_t;
    using Any_Callable_Wrapper = std::function<std::any(std::vector<std::any>)>;
    using precedence           = std::unordered_map<Symbol_Name, Operator_Properties>;

    struct Type
    {
        using type = Any_Callable_Wrapper;

        template<traits::Type_Object_Value_Semantic T>
        static Type
        make_type(std::optional<std::decay_t<T>> default_ = std::nullopt);

        Type(type default_value, std::type_index type_id)
            : get_default_value_(std::move(default_value))
            , type_id_(type_id)
        {
        }

        Type(const Type &)            = default;
        Type(Type &&)                 = default;
        Type &operator=(const Type &) = default;
        Type &operator=(Type &&)      = default;

        std::any
        get_default(void) const
        {
            return get_default_value_({});
        }

        std::type_index
        type(void) const
        {
            return type_id_;
        }

      protected:

        type            get_default_value_;
        std::type_index type_id_;
    };

    struct Functional
    {
        using type = Any_Callable_Wrapper;
        using sign = Proc_Signature;

        template<traits::Type_Callable T>
        Functional(T &&body)
            : signature_(Proc_Signature::from<T>())
        {
            type body_ = [callable = std::forward<T>(body)](std::vector<std::any> args) -> std::any
            {
                using Callable         = std::decay_t<T>;
                using Args             = boost::callable_traits::args_t<Callable>;
                using Result           = boost::callable_traits::return_type_t<Callable>;
                constexpr size_t arity = std::tuple_size_v<Args>;

                // Проверяем количество аргументов
                if (args.size() != arity)
                {
                    throw std::runtime_error("Argument count mismatch");
                }

                // Распаковываем аргументы из std::any в кортеж
                try
                {
                    Args unpacked_args = [&]<size_t... I>(std::index_sequence<I...>)
                    {
                        return std::make_tuple(
                                std::any_cast<std::tuple_element_t<I, Args>>(args[I])...
                        );
                    }(std::make_index_sequence<arity> {});

                    // Вызываем функцию с распакованными аргументами
                    if constexpr (std::is_void_v<Result>)
                    {
                        std::apply(callable, unpacked_args);
                        return std::any {};
                    }
                    else
                    {
                        return std::apply(callable, unpacked_args);
                    }
                }
                catch (std::bad_any_cast &e)
                {
                    throw std::runtime_error("Argument type error during functional call.");
                }
            };

            data_ = std::move(body_);
        }

        Functional(const Functional &)            = default;
        Functional(Functional &&)                 = default;
        Functional &operator=(const Functional &) = default;
        Functional &operator=(Functional &&)      = default;

        template<typename... Args>
            requires((traits::Type_Object_Value_Semantic<Args> && std::is_constructible_v<std::any, Args>, ...))
        std::any
        operator()(Args... args) const
        {
            constexpr std::size_t arity = sizeof...(Args);
            std::vector<std::any> arguments;
            arguments.reserve(arity);

            ((arguments.push_back(std::make_any(args))), ...);

            return operator()(std::move(arguments));
        }

        std::any
        operator()(std::vector<std::any> args) const
        {
            return body_(std::move(args));
        }

        const sign &
        signature(void) const
        {
            return signature_;
        }

      protected:

        type body_;
        sign signature_;
    };

    struct Data_Object
    {
        Data_Object(std::any value, Symbol_Key type)
            : data_(value)
            , type_(type)
        {
        }

        Data_Object(const Data_Object &)            = default;
        Data_Object(Data_Object &&)                 = default;
        Data_Object &operator=(const Data_Object &) = default;
        Data_Object &operator=(Data_Object &&)      = default;

        const std::any &
        data(void) const
        {
            return data_;
        }

        std::any &
        data(void)
        {
            return data_;
        }

        Symbol_Key
        type(void) const
        {
            return type_;
        }

      protected:

        std::any   data_;
        Symbol_Key type_;
    };

    struct Symbol
    {
        using Object = std::variant<Type, Functional, Data_Object>;

        Symbol(Object value)
            : obj_(value)
        {
        }

        Symbol(const Symbol &)            = default;
        Symbol(Symbol &&)                 = default;
        Symbol &operator=(const Symbol &) = default;
        Symbol &operator=(Symbol &&)      = default;

        Object &
        variant(void)
        {
            return obj_;
        }

        const Object &
        variant(void) const
        {
            return obj_;
        }

        template<typename T>
        const T *
        get_if(void) const
        {
            return std::get_if<T>(&obj_);
        }

        template<typename T>
        T *
        get_if(void)
        {
            return std::get_if<T>(&obj_);
        }

      protected:

        Object obj_;
    };

  public:

    void
    link_static(const Module &m)
    {
#if 0 // TODO: implement
        identifiers.insert(m.identifiers.begin(), m.identifiers.end());
        prefix_ops.insert(m.prefix_ops.begin(), m.prefix_ops.end());
        infix_ops.insert(m.infix_ops.begin(), m.infix_ops.end());
#endif
    }

    void
    link_shared(const Module &m)
    {
#if 0
        linked.push_front(std::ref(m));
#endif
    }

  public:

    precedence
    operators_info(void) const
    {
        return op_properties_;
    }

  public:

    const Symbol *
    find_symbol(std::string name, Symbol_Key key) const
    {
        using Result = const Symbol *;
        using namespace std::string_literals;

        Result result;

        std::ranges::find

        if (is_search_for_type && types.count(name))
        {
            result.emplace(types.at(name));
        }
        else if (is_search_for_func && identifiers.count(name))
        {
            result.emplace(identifiers.at(name));
        }
        else if (is_search_for_unop && prefix_ops.count(name))
        {
            result.emplace(prefix_ops.at(name));
        }
        else if (is_search_for_binop && infix_ops.count(name))
        {
            result.emplace(infix_ops.at(name));
        }
        else if (!linked.empty())
        {
            for (auto &&module_ : linked | std::views::reverse)
            {
                auto res = module_.get().find_symbol(name, filter_by);
                if (res.has_value())
                {
                    result.emplace(res.value());
                }
                else
                {
                    continue;
                }
            }
        }
        return nullptr;
    }

  public:

    template<traits::Type_Object_Value_Semantic T>
    Module &
    bind_type(std::string name)
    {
        Symbol_Key key {.name = name, .kind = Symbol_Kind::Type};

        auto res = std::ranges::find(lookup_cache_, key);

        if (res != lookup_cache_.end())
        {
            throw std::invalid_argument("type already exists: " + name);
        }
        else
        {
            symbols.insert_or_assign(Symbol {Type::make_type<T>()});

            size_t sym_idx = symbols.size() - 1;
            lookup_cache_.insert_or_assign(std::move(key), sym_idx);
        }

        return *this;
    }

    template<traits::Type_Callable T>
    Module &
    bind_func(std::string name, T &&value)
    {
        auto       sign = Proc_Signature::from<T>();
        Symbol_Key key {.name = name, .kind = Symbol_Kind::Func, .overload = sign};

        auto declaration_str = std::accumulate(
                sign.args.cbegin(), sign.args.cend(), std::string {}, [](auto acc, auto &&elem)
                {
                    return (acc.empty()) ? compiler::demangle(elem.name()) : acc + ", " + compiler::demangle(elem.name());
                }
        );

        auto res = std::ranges::find(lookup_cache_, key);

        if (res != lookup_cache_.end())
        {
            throw std::invalid_argument("overload already exists: " + "function " + name + "(" + declaration_str + ")");
        }
        else
        {
            std::cout << "bind function: " << name << "(" << declaration_str << ")" << std::endl;

            symbols.emplace_back(Symbol {Functional {std::move(sign), std::forward<T>(value)}});

            size_t sym_idx = symbols.size() - 1;
            lookup_cache_.insert_or_assign(std::move(key), sym_idx);
        }

        return *this;
    }

    template<traits::Type_Callable T>
    Module &
    bind_op(std::string name, Operator_Properties properties, T &&value)
    {
        Symbol_Key  key;
        auto        sign = Proc_Signature::from<T>();
        std::string opkind_keyword;
        if (Operator_Properties::Kind::Infix == properties.kind)
        {
            key = Symbol_Key
            {
                .name     = name,
                .kind     = Symbol_Kind::Infix,
                .overload = sign;
            };

            opkind_keyword = "infix"
        }
        else if (Operator_Properties::Kind::Prefix == properties.kind)
        {
            key = Symbol_Key
            {
                .name     = name,
                .kind     = Symbol_Kind::Prefix,
                .overload = sign;
            };

            opkind_keyword = "prefix"
        }
        else // if (Operator_Properties::Kind::Postfix == properties.kind)
        {
            key = Symbol_Key
            {
                .name     = name,
                .kind     = Symbol_Kind::Postfix,
                .overload = sign;
            };

            opkind_keyword = "postfix"
        }

        auto declaration_str = std::accumulate(
                sign.args.cbegin(), sign.args.cend(), std::string {}, [](auto acc, auto &&elem)
                {
                    return (acc.empty()) ? compiler::demangle(elem.name()) : acc + ", " + compiler::demangle(elem.name());
                }
        );

        auto res = std::ranges::find(lookup_cache_, key);

        if (res != lookup_cache_.end())
        {
            throw std::invalid_argument("overload already exists: " + opkind_keyword + " " + name + "(" + declaration_str + ")");
        }
        else
        {
            std::cout << "bind " << opkind_keyword << ": " << name << "(" << declaration_str << ")" << std::endl;

            symbols.emplace_back(Symbol {Functional {std::move(sign), std::forward<T>(value)}});

            size_t sym_idx = symbols.size() - 1;
            op_properties_.insert_or_assign(std::make_pair(name, properties));
            lookup_cache_.insert_or_assign(std::move(key), sym_idx);
        }

        return *this;
    }

    template<traits::Type_Object_Value_Semantic T>
    Module &
    bind_var(std::string name, T &&value)
    {
        identifiers.insert_or_assign(
                name,
                Symbol {
                        Data_Object {
                                [value = std::forward<T>(value)](std::vector<std::any> args) -> std::any
                                {
                                    return value;
                                }
                        }
                }
        );
        return *this;
    }

  public:

    void
    clear(void)
    {
        symbols.clear();
        op_properties_.clear();
        lookup_cache_.clear();
    }

  protected:

    std::vector<Symbol>              symbols;
    precedence                       op_properties_; // should be constant by unique operator name!
    std::map<Symbol_Key, Symbol_Idx> lookup_cache_;

    std::list<std::reference_wrapper<const Module>> linked;
};

template<traits::Type_Object_Value_Semantic T>
inline Module::Type
Module::Type::make_type(std::optional<std::decay_t<T>> default_)
{
    using TypePure = std::decay_t<T>;

    TypePure init_value;

    if constexpr (std::is_integral_v<TypePure> || std::is_ariphmetic_v<TypePure>)
    {
        init_value = default_.value_or(TypePure {0});
    }
    else
    {
        init_value = default_.value_or(TypePure {});
    }

    Type::type initializer = [value_ = std::any(init_value)](std::vector<std::any>) -> std::any
    {
        return value_;
    };

    return Type {
            std::move(initializer),
            std::type_index {typeid(TypePure)}
    };
}

} // namespace kaleidoscope

namespace std
{
template<>
struct hash<::kaleidoscope::Module::Symbol_Key>
{
    size_t
    operator()(const ::kaleidoscope::Module::Symbol_Key &key) const
    {
        size_t seed = 0;
        boost::hash_combine(seed, key.name);
        boost::hash_combine(seed, static_cast<size_t>(key.kind));
        if (key.overload)
        {
            boost::hash_combine(seed, key.overload->fingerprint());
        }
        return seed;
    }
};
} // namespace std
