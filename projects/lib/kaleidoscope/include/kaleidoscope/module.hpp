#pragma once

#include <kaleidoscope/ast.hpp>

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

namespace kaleidoscope
{

struct Module
{
    struct Symbol; // forward decl

    enum class Function_Filter
    {
        Any,
        Func,
        BinOp,
        UnOp
    };

    struct operator_properties
    {
        using Precedence = size_t;

        enum class Associativity : uint8_t
        {
            Left,
            Right
        };

        enum class Kind : uint8_t
        {
            Unary,
            Binary
        };

        Kind          kind          = Kind::Binary;
        Associativity associativity = Associativity::Left;
        Precedence    precedence    = 0; /**< lesser is higher */
    };

    struct Data_Object
    {
        Data_Object(std::any value)
            : data_(value)
        {
        }

        Data_Object(const Data_Object &)            = default;
        Data_Object(Data_Object &&)                 = default;
        Data_Object &operator=(const Data_Object &) = default;
        Data_Object &operator=(Data_Object &&)      = default;

        std::any
        value(void) const
        {
            return data_;
        }

        const std::any &
        data(void) const
        {
            return data_;
        }

      protected:

        std::any data_;
    };

    struct Functional
    {
        using type = std::function<std::any(std::vector<std::any>)>;

        Functional(type body)
            : data_(body)
        {
        }

        Functional(const Functional &)            = default;
        Functional(Functional &&)                 = default;
        Functional &operator=(const Functional &) = default;
        Functional &operator=(Functional &&)      = default;

        template<typename... Args>
        std::any
        value(Args... args)
        {
            std::vector<std::any> arguments;

            ((arguments.push_back(std::make_any(args))), ...);

            return std::any_cast<type>(data_)(std::move(arguments));
        }

        const std::any &
        data(void) const
        {
            return data_;
        }

      protected:

        std::any data_;
    };

    struct Operator
    {
        using type = std::function<std::any(std::vector<std::any>)>;

        Operator(operator_properties properties, type body)
            : props_(properties)
            , data_(body)
        {
        }

        Operator(const Operator &)            = default;
        Operator(Operator &&)                 = default;
        Operator &operator=(const Operator &) = default;
        Operator &operator=(Operator &&)      = default;

        template<typename... Args>
        std::any value(Args... args);

        const std::any &
        data(void) const
        {
            return data_;
        }

        const operator_properties &
        props() const
        {
            return props_;
        }

      protected:

        operator_properties props_;
        std::any            data_;
    };

    using precedence  = std::unordered_map<std::string, operator_properties>;
    using Symbol_Name = std::string;

    struct Symbol
    {
        using Object = std::variant<Data_Object, Functional, Operator>;

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

        const std::any &
        value()
        {
            const std::any *result;
            std::visit(
                    [&](auto &&v)
                    {
                        result = &v.data();
                    },
                    obj_
            );
            return *result;
        }

      protected:

        Object obj_;
    };

  public:

    void
    link_static(const Module &m)
    {
        identifiers.insert(m.identifiers.begin(), m.identifiers.end());
        unary_ops.insert(m.unary_ops.begin(), m.unary_ops.end());
        binary_ops.insert(m.binary_ops.begin(), m.binary_ops.end());
    }

    void
    link_shared(const Module &m)
    {
        linked.push_front(std::ref(m));
    }

    precedence
    collect_operators_info(void) const
    {
        auto current_module = std::accumulate(
                binary_ops.cbegin(),
                binary_ops.cend(),
                precedence {},
                [](auto acc, const auto &pair)
                {
                    if (auto op = std::get_if<Operator>(&pair.second.variant()))
                    {
                        acc.emplace(std::make_pair(pair.first, op->props()));
                    }
                    else
                    {
                        // do nothing
                    }
                    return acc;
                }
        );

        for (auto &&m : linked | std::ranges::views::reverse)
        {
            auto result = m.get().collect_operators_info();
            current_module.insert(result.begin(), result.end());
        }

        return current_module;
    }

    const std::any &
    at(const std::string &symbol)
    {
        const std::any *result = nullptr;

        std::visit([&](auto &&v)
                   {
                       result = &v.data();
                   },
                   identifiers.at(symbol).variant());
        return *result;
    }

    const Functional::type &
    get_func(const std::string &name)
    {
        using namespace std::string_literals;
        auto res = find_symbol(name, Function_Filter::Func);

        if (res.has_value())
        {
            if (auto sym = std::get_if<Functional>(&res.value().get().variant()))
            {
                return *std::any_cast<Functional::type>(&sym->data());
            }
            else
            {
                throw std::runtime_error("Symbol "s + name + " is not a function");
            }
        }
        else
        {
            throw std::runtime_error("Symbol "s + name + " is undefined");
        }
    };

    const Functional::type &
    get_binop(const std::string &name)
    {
        using namespace std::string_literals;
        auto res = find_symbol(name, Function_Filter::BinOp);

        if (res.has_value())
        {
            if (auto sym = std::get_if<Operator>(&res.value().get().variant()))
            {
                return *std::any_cast<Functional::type>(&sym->data());
            }
            else
            {
                throw std::runtime_error("Symbol "s + name + " is not a binary operator");
            }
        }
        else
        {
            throw std::runtime_error("Symbol "s + name + " is undefined");
        }
    };

    const Functional::type &
    get_unop(const std::string &name)
    {
        using namespace std::string_literals;
        auto res = find_symbol(name, Function_Filter::UnOp);

        if (res.has_value())
        {
            if (auto sym = std::get_if<Operator>(&res.value().get().variant()))
            {
                return *std::any_cast<Functional::type>(&sym->data());
            }
            else
            {
                throw std::runtime_error("Symbol "s + name + " is not an unary operator");
            }
        }
        else
        {
            throw std::runtime_error("Symbol "s + name + " is undefined");
        }
    };

    std::optional<std::reference_wrapper<const Symbol>>
    find_symbol(std::string name, Function_Filter filter_by = Function_Filter::Any) const
    {
        using Result = std::optional<std::reference_wrapper<const Symbol>>;
        using namespace std::string_literals;

        Result result;

        bool is_search_for_func  = (Function_Filter::Any == filter_by) || (Function_Filter::Func == filter_by);
        bool is_search_for_unop  = (Function_Filter::Any == filter_by) || (Function_Filter::UnOp == filter_by);
        bool is_search_for_binop = (Function_Filter::Any == filter_by) || (Function_Filter::BinOp == filter_by);

        if (is_search_for_func && identifiers.count(name))
        {
            result.emplace(identifiers.at(name));
        }
        else if (is_search_for_unop && unary_ops.count(name))
        {
            result.emplace(unary_ops.at(name));
        }
        else if (is_search_for_binop && binary_ops.count(name))
        {
            result.emplace(binary_ops.at(name));
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
        else
        {
            result = std::nullopt;
        }

        return result;
    }

    Module &
    bind_func(std::string name, Functional::type value)
    {
        identifiers.insert_or_assign(name, Symbol {Functional {value}});
        return *this;
    }

    Module &
    bind_var(std::string name, double value)
    {
        identifiers.insert_or_assign(
                name,
                Symbol {
                        Functional {
                                [=](std::vector<std::any> args) -> std::any
                                {
                                    return value;
                                }
                        }
                }
        );
        return *this;
    }

    Module &
    bind_op(std::string name, Operator value)
    {
        if (operator_properties::Kind::Binary == value.props().kind)
        {
            binary_ops.emplace(name, value);
        }
        else // if (operator_properties::Kind::Unary == value.props().kind)
        {
            unary_ops.emplace(name, value);
        }

        return *this;
    }

    void
    clear(void)
    {
        identifiers.clear();
        unary_ops.clear();
        binary_ops.clear();
    }

  protected:

    std::unordered_map<Symbol_Name, Symbol> identifiers;
    std::unordered_map<Symbol_Name, Symbol> unary_ops;
    std::unordered_map<Symbol_Name, Symbol> binary_ops;

    std::list<std::reference_wrapper<const Module>> linked;
};

} // namespace kaleidoscope
