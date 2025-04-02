#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_uint8_t.hpp>

#include <string>
#include <iostream>
#include <cassert>

int
main(int argc, const char *argv[])
{
    std::string              input {"1; 2; 3"};
    std::span<const uint8_t> input_span {reinterpret_cast<uint8_t *>(input.data()), input.size()};

    auto result = kaleidoscope::Parser::parse(input_span.begin(), input_span.end());
    for (auto &&node : result.statements)
    {
        if (auto num = dynamic_cast<kaleidoscope::ast::Lexeme_Numeric *>(node.get()))
        {
            std::visit(
                    [&](auto &&v)
                    {
                        std::cout << v << std::endl;
                    },
                    num->value
            );
        }
    }
    return 0;
}
