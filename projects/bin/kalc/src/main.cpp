#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_uint8_t.hpp>
#include <kaleidoscope/ast/stringify.hpp>

#include <string>
#include <iostream>
#include <cassert>

int
main(int argc, const char *argv[])
{
    using namespace kaleidoscope::ast::utils;

    std::string              input {"a; 1"};
    std::span<const uint8_t> input_span {reinterpret_cast<uint8_t *>(input.data()), input.size()};

    auto result = kaleidoscope::Parser::parse(input_span.begin(), input_span.end());
    for (auto &&node : result.statements)
    {
        if (auto num = dynamic_cast<kaleidoscope::ast::Lexeme_Numeric *>(node.get()))
        {
            std::cout << Stringify {}.visit(*node.get()).result() << std::endl;
        }
        else if (auto var = dynamic_cast<kaleidoscope::ast::Variable *>(node.get()))
        {
            std::cout << to_string(*var) << std::endl;
        }
        else if (auto expr = dynamic_cast<kaleidoscope::ast::Precedence_Agnostic_Expr *>(node.get()))
        {
            std::cout << Stringify {}.visit(*(expr->first.get())).result() << std::endl;
        }
    }
    return 0;
}
