#pragma once
#include <kaleidoscope/ast.hpp>
#include <kaleidoscope/ast/visitor.hpp>

#include <string_view>
#include <numeric>
#include <type_traits>
#include <format>

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
to_string<Literal_Numeric>(const Literal_Numeric &node)
{
    std::string result;
    std::visit(
            [&](auto &&v)
            {
                result = std::format("{:g}", v);
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
to_string<Function_Declaration>(const Function_Declaration &node)
{
    auto arg_list =
            std::accumulate(
                    node.args.begin(),
                    node.args.end(),
                    std::string {},
                    [](auto &&acc, auto &arg)
                    {
                        acc += (acc.empty()) ? arg : (" " + arg);
                        return acc;
                    }
            );
    return "(defun " + node.name + " (" + arg_list + "))";
}

} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{

struct Stringify : public Visitor_Node_CRTP<Stringify, ast::Node>
{
    Stringify(void)
    {
        register_handler<ast::Literal_Numeric>(
                [&](const ast::Literal_Numeric &obj)
                {
                    value = to_string(obj);
                }
        );

        register_handler<ast::Literal_String>(
                [&](const ast::Literal_String &obj)
                {
                    value = "\"" + obj.value + "\"";
                }
        );

        register_handler<ast::Variable>(
                [&](const ast::Variable &obj)
                {
                    value = to_string(obj);
                }
        );

        register_handler<ast::Function_Declaration>(
                [&](const ast::Function_Declaration &obj)
                {
                    value = to_string(obj);
                }
        );

        register_handler<ast::Function_Defenition>(
                [&](const ast::Function_Defenition &obj)
                {
                    std::string result {this->visit(*obj.prototype).result()};
                    result.pop_back(); // удалить завершающую ')' у прототипа
                    for (auto &&stmt : obj.body)
                    {
                        result += " " + std::string(this->visit(*stmt).result());
                    }
                    value = result + ")";
                }
        );

        register_handler<ast::Functional_Call>(
                [&](const ast::Functional_Call &obj)
                {
                    std::string result  = "(" + obj.callee + " ";
                    result             += std::accumulate(
                            obj.args.begin(),
                            obj.args.end(),
                            std::string {},
                            [this](auto acc, auto &arg)
                            {
                                acc += (acc.empty()) ? std::string {this->visit(*arg).result()} : " " + std::string {this->visit(*arg).result()};
                                return acc;
                            }
                    );
                    value = result + ")";
                }
        );

        register_handler<ast::Precedence_Agnostic_Expr>(
                [&](const ast::Precedence_Agnostic_Expr &obj)
                {
                    std::string result {this->visit(*obj.first).result()};
                    result = "(" + result;
                    for (auto &&[op, expr] : obj.operations)
                    {
                        result += " " + op + " " + std::string {this->visit(*expr).result()};
                    }
                    value = result + ")";
                }
        );

        register_handler<ast::Operation_Postfix>(
                [&](const ast::Operation_Postfix &obj)
                {
                    std::string result {
                            "(postfix " + obj.op + ")"
                            + " " + std::string(this->visit(*obj.operand).result())
                    };

                    value = "(" + result + ")";
                }
        );

        register_handler<ast::Operation_Prefix>(
                [&](const ast::Operation_Prefix &obj)
                {
                    std::string result {
                            obj.op
                            + " " + std::string(this->visit(*obj.operand).result())
                    };

                    value = "(" + result + ")";
                }
        );

        register_handler<ast::Operation_Infix>(
                [&](const ast::Operation_Infix &obj)
                {
                    std::string result {
                            obj.op
                            + " " + std::string(this->visit(*obj.lhs).result())
                            + " " + std::string(this->visit(*obj.rhs).result())
                    };

                    value = "(" + result + ")";
                }
        );

        register_handler<ast::Type_Declaration>(
                [&](const ast::Type_Declaration &obj)
                {
                    value = obj.name;
                }
        );

        register_handler<ast::Data_Object_Definition_List>(
                [&](const ast::Data_Object_Definition_List &obj)
                {
                    auto names =
                            std::accumulate(
                                    obj.names.begin(), obj.names.end(), std::string {}, [](auto acc, auto &&name)
                                    {
                                        return (acc.empty()) ? name : acc + " " + name;
                                    }
                            );
                    std::string result {
                            "(type " + std::string(this->visit(*obj.type).result()) + ") " + names
                    };

                    value = "(" + std::string((obj.is_mutable) ? "var" : "let") + " " + result + ")";
                }
        );
    }

    std::string_view
    result(void)
    {
        return value;
    }

  protected:

    std::string value;
};

template<>
std::string
to_string<Node>(const Node &node)
{
    return std::string {Stringify {}.visit(node).result()};
}

} // namespace kaleidoscope::ast::utils
