#pragma once

#include <kaleidoscope/ast/visitor.hpp>

#include <concepts>
#include <type_traits>

namespace kaleidoscope::traits
{
template<typename T>
struct Clone;

template<traits::Ast_Node T>
struct Clone<std::unique_ptr<T>>
{
    using type = std::unique_ptr<T>;
};

} // namespace kaleidoscope::traits

namespace kaleidoscope::ast::utils
{

struct replicator : Visitor_Node_CRTP<replicator, ast::Node, std::unique_ptr<ast::Node>>
{
    replicator(void);
};

} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{

inline replicator::replicator(void)
{
    register_handler<ast::Literal_Numeric>(
            [&](const ast::Literal_Numeric &node)
            {
                return std::make_unique<ast::Literal_Numeric>(node);
            }
    );

    register_handler<ast::Literal_String>(
            [&](const ast::Literal_String &node)
            {
                return std::make_unique<ast::Literal_String>(node);
            }
    );

    register_handler<ast::Variable>(
            [&](const ast::Variable &node)
            {
                return std::make_unique<ast::Variable>(node);
            }
    );

    register_handler<ast::Function_Declaration>(
            [&](const ast::Function_Declaration &node)
            {
                return std::make_unique<ast::Function_Declaration>(node);
            }
    );

    register_handler<ast::Function_Defenition>(
            [&](const ast::Function_Defenition &node)
            {
                return std::make_unique<ast::Function_Defenition>(
                        std::unique_ptr<ast::Function_Declaration>(
                                static_cast<ast::Function_Declaration *>(
                                        visit(*node.prototype).value().release()
                                )
                        ),
                        std::accumulate(
                                node.body.begin(),
                                node.body.end(),
                                ast::Function_Defenition::Body {},
                                [&](auto acc, auto &&elem)
                                {
                                    acc.emplace_back(visit(*elem).value());
                                    return acc;
                                }
                        )
                );
            }
    );

    register_handler<ast::Functional_Call>(
            [&](const ast::Functional_Call &node)
            {
                return std::make_unique<ast::Functional_Call>(
                        node.callee,
                        std::accumulate(
                                node.args.begin(),
                                node.args.end(),
                                ast::Functional_Call::Args {},
                                [&](auto acc, auto &&el)
                                {
                                    acc.emplace_back(visit(*el).value());
                                    return acc;
                                }
                        )
                );
            }
    );

    register_handler<ast::Operation_Postfix>(
            [&](const ast::Operation_Postfix &node)
            {
                return std::make_unique<ast::Operation_Postfix>(
                        node.op,
                        visit(*node.operand).value()
                );
            }
    );

    register_handler<ast::Operation_Prefix>(
            [&](const ast::Operation_Prefix &node)
            {
                return std::make_unique<ast::Operation_Prefix>(
                        node.op,
                        visit(*node.operand).value()
                );
            }
    );

    register_handler<ast::Operation_Infix>(
            [&](const ast::Operation_Infix &node)
            {
                return std::make_unique<ast::Operation_Infix>(
                        node.op,
                        visit(*node.lhs).value(),
                        visit(*node.rhs).value()
                );
            }
    );

    register_handler<ast::Precedence_Agnostic_Expr>(
            [&](const ast::Precedence_Agnostic_Expr &node)
            {
                return std::make_unique<ast::Precedence_Agnostic_Expr>(
                        visit(*node.first).value(),
                        std::accumulate(
                                node.operations.begin(),
                                node.operations.end(),
                                ast::Precedence_Agnostic_Expr::Operations {},
                                [&](auto acc, auto &&el)
                                {
                                    acc.emplace_back(el.first, visit(*el.second).value());
                                    return acc;
                                }
                        )
                );
            }
    );

    register_handler<ast::Type_Declaration>(
            [&](const ast::Type_Declaration &node)
            {
                return std::make_unique<ast::Type_Declaration>(node);
            }
    );

    register_handler<ast::Data_Object_Definition_List>(
            [&](const ast::Data_Object_Definition_List &node)
            {
                return std::make_unique<ast::Data_Object_Definition_List>(
                        node.names,
                        node.is_mutable,
                        (node.type) ? visit(*node.type).value() : nullptr
                );
            }
    );
}

} // namespace kaleidoscope::ast::utils
