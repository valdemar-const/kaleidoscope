#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_char.hpp>
#include <kaleidoscope/ast/stringify.hpp>

#include <string>
#include <iostream>
#include <cassert>

int
main(int argc, const char *argv[])
{
    using namespace kaleidoscope::ast::utils;

    std::string input {
            R"KALEIDOSCOPE(
                def foo(a, b, c);
                g + 7;
                a + (b - c * foo(1 %=% foo(3, 2, a - c), 2, 3) &% g) %% f;
                1;
                def foo(a, b, c)
                    a - b * c
            )KALEIDOSCOPE"
    };
    std::span<char> input_span {reinterpret_cast<char *>(input.data()), input.size()};

    auto result = kaleidoscope::Parser::parse(input_span.begin(), input_span.end());
    for (auto &&node : result.statements)
    {
        std::cout << Stringify {}.visit(*node).result() << std::endl;
    }
    return 0;
}
