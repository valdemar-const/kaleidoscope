#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_char.hpp>
#include <kaleidoscope/ast/stringify.hpp>

#include <kaleidoscope/state.hpp>

#include <string>
#include <iostream>

#include <cassert>

int
main(int argc, const char *argv[])
{
    using namespace kaleidoscope::ast::utils;
    using namespace std::string_literals;

    std::string input =
            R"KALEIDOSCOPE(
                def foo(a, b, c);
                g + 7;
                a + (b - c * foo(1 + foo(3, 2, a - c), 2, 3) - g) / f;
                1;
                def foo(a, b, c) a - b * c
            )KALEIDOSCOPE"s;

    {
        std::span<char> input_span {reinterpret_cast<char *>(input.data()), input.size()};

        auto result = kaleidoscope::Parser::parse(input_span.begin(), input_span.end());
        for (auto &&node : result.statements)
        {
            std::cout << to_string(*node) << std::endl;
        }
    }

    // --------

    {
        kaleidoscope::state context;
        context.source("42"s);     // apply/update symbols, build toplevel anonimus func(args) { return 42; }
        double result = context(); // syntx shugar for:
        assert((context.eval("42"s) == result));
        std::cout << "result = " << result << std::endl;
    }

    return 0;
}
