#pragma once
#include <kaleidoscope/ast.hpp>
#include <kaleidoscope/ast/visitor.hpp>

#include <string_view>
#include <type_traits>

namespace kaleidoscope::ast::utils
{
template<typename T>
    requires requires { std::is_base_of_v<ast::Node, T>; }
std::string to_string(const T &node);
} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{
template<>
std::string
to_string<Lexeme_Numeric>(const Lexeme_Numeric &node)
{
    std::string result;
    std::visit(
            [&](auto &&v)
            {
                result = std::to_string(v);
            },
            node.value
    );
    return result;
}

template<>
std::string
to_string<Variable>(const Variable &node)
{
    return node.name;
}

template<>
std::string
to_string<Precedence_Agnostic_Expr>(const Precedence_Agnostic_Expr &node)
{
    return "to_string(expr) <- not implemented!";
}

} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{

struct Stringify : public Visitor_Node_CRTP<Stringify, ast::Node>
{
    Stringify(void)
    {
        register_handler<ast::Lexeme_Numeric>(
                [&](const ast::Lexeme_Numeric &obj)
                {
                    value = to_string(obj);
                }
        );
    }

    const std::string_view
    result(void)
    {
        return value;
    }

  protected:

    std::string value;
};

} // namespace kaleidoscope::ast::utils
