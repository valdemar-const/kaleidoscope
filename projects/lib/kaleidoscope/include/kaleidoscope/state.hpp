#pragma once

#include <type_traits>
#include <concepts>

#include <iostream>

namespace kaleidoscope
{
struct state
{
  public:

    struct result
    {
        template<typename T>
        operator T(void)
        {
            return T {};
        }

        operator double(void)
        {
            return 42;
        }

        template<typename T>
        bool operator==(const T &rhs);
    };

    struct symbol
    {
        result
        operator()(void)
        {
            return {};
        }
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

  public:

    state(void)
    {
        add_operator(
                "+",
                operator_properties {
                        .kind          = operator_properties::Kind::Binary,
                        .associativity = operator_properties::Associativity::Left,
                        .precedence    = 40
                },
                [](const int &lhs, const int &rhs)
                {
                    return lhs + rhs;
                }
        );
    }

  public:

    template<typename F>
    state &
    add_operator(std::string name, operator_properties props, F impl)
    {
        return *this;
    }

  public:

    state &
    source(std::string_view src)
    {
        return *this;
    }

    result
    eval(std::string_view src)
    {
        return {};
    }

  public:

    symbol
    operator[](std::string_view symb)
    {
        return {};
    }

    result
    operator()(void)
    {
        return {};
    }

  protected:
};
} // namespace kaleidoscope

template<typename T>
bool
operator==(const kaleidoscope::state::result &lhs, const T &rhs);

template<>
bool // clang-format off
kaleidoscope::state::result::operator==<double>(const double &rhs) // clang-format on
{
    double a = *this;
    return a == rhs;
}
