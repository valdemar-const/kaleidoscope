#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_char.hpp>
#include <kaleidoscope/ast/stringify.hpp>
#include <kaleidoscope/ast/precedence.hpp>
#include <kaleidoscope/ast/iterator.hpp>

#include <kaleidoscope/state.hpp>

#include <string>
#include <iostream>

#include <cassert>

int
main(int argc, const char *argv[])
{
    using namespace kaleidoscope::ast::utils;
    using namespace std::string_literals;
    using Kind          = kaleidoscope::state::operator_properties::Kind;
    using Associativity = kaleidoscope::state::operator_properties::Associativity;

    // Given

    const precedence::Bin_Op_Precedence operators = {
            std::make_pair(
                    "**",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Right,
                            .precedence    = 0
                    }
            ),
            std::make_pair(
                    "*",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 10
                    }
            ),
            std::make_pair(
                    "/",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 10
                    }
            ),
            std::make_pair(
                    "+",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 20
                    }
            ),
            std::make_pair(
                    "-",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 20
                    }
            )
    };

#if 1
    {
        std::string input =
            R"KALEIDOSCOPE(
                def foo(a, b, c);
                1;
                1 - 2 - 3 ** 5 ** 6 - 4;
                5 + 5 * 2 - 1;
                g + -(7 + b) * -1;
                a + (b - c * foo(1 + foo(3, 2, a - c), 2, 3) - g) / f;
                1;
                def foo(a, b, c)
                    (a - b) * c;
                    a - b * c
                end
            )KALEIDOSCOPE"s;

        auto result = kaleidoscope::Parser::parse(input.begin(), input.end());

        precedence {operators}(result);

        for (auto &&node : result.statements)
        {
            std::cout << to_string(*node) << std::endl;
        }
    }
#endif
#if 1
    {
        std::string input {"1 ** 2 ** 3 - 4"};

        auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
        precedence {operators}(result);
        for (auto &&node : result.statements)
        {
            std::cout << to_string(*node) << std::endl;
        }
    }
#endif
#if 1
    {
        kaleidoscope::state context;

        context.source("42"s);     // apply/update symbols, build toplevel anonymous func(args) { return 42; }
        double result = context(); // syntax sugar for: context.call().as<double>();
        assert((context.eval("42"s) == result));

        std::cout << "result = " << result << std::endl;
    }
#endif

    return 0;
}
