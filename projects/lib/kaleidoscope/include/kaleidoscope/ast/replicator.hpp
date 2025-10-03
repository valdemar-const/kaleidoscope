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

struct replicator : Visitor_Node_CRTP<replicator, ast::Node>
{
    replicator(void);

    std::unique_ptr<ast::Node> result(void);

  protected:

    std::unique_ptr<ast::Node> result_;
};

} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{

inline replicator::replicator(void)
{
    register_handler<ast::Lexeme_Numeric>(
            [&](const ast::Lexeme_Numeric &node)
            {
                result_.reset(new ast::Lexeme_Numeric(node));
            }
    );

    register_handler<ast::Variable>(
            [&](const ast::Variable &node)
            {
                result_.reset(new ast::Variable(node));
            }
    );

    register_handler<ast::Function_Declaration>(
            [&](const ast::Function_Declaration &node)
            {
                result_.reset(new ast::Function_Declaration(node));
            }
    );

    register_handler<ast::Function_Defenition>(
            [&](const ast::Function_Defenition &node)
            {
                result_.reset(
                        new ast::Function_Defenition(
                                std::unique_ptr<ast::Function_Declaration>(
                                        static_cast<ast::Function_Declaration *>(
                                                visit(*node.prototype).result().release()
                                        )
                                ),
                                std::accumulate(
                                        node.body.begin(),
                                        node.body.end(),
                                        ast::Function_Defenition::Body {},
                                        [&](auto acc, auto &&elem)
                                        {
                                            acc.emplace_back(visit(*elem).result());
                                            return acc;
                                        }
                                )
                        )
                );
            }
    );

    register_handler<ast::Functional_Call>(
            [&](const ast::Functional_Call &node)
            {
                result_.reset(new ast::Functional_Call(
                        node.callee,
                        std::accumulate(
                                node.args.begin(),
                                node.args.end(),
                                ast::Functional_Call::Args {},
                                [&](auto acc, auto &&el)
                                {
                                    acc.emplace_back(visit(*el).result());
                                    return acc;
                                }
                        )
                ));
            }
    );

    register_handler<ast::Operation_Unary>(
            [&](const ast::Operation_Unary &node)
            {
                result_.reset(new ast::Operation_Unary(
                        node.op,
                        visit(*node.operand).result()
                ));
            }
    );

    register_handler<ast::Operation_Binary>(
            [&](const ast::Operation_Binary &node)
            {
                result_.reset(new ast::Operation_Binary(
                        node.op,
                        visit(*node.lhs).result(),
                        visit(*node.rhs).result()
                ));
            }
    );

    register_handler<ast::Precedence_Agnostic_Expr>(
            [&](const ast::Precedence_Agnostic_Expr &node)
            {
                result_.reset(new ast::Precedence_Agnostic_Expr(
                        visit(*node.first).result(),
                        std::accumulate(
                                node.operations.begin(),
                                node.operations.end(),
                                ast::Precedence_Agnostic_Expr::Operations {},
                                [&](auto acc, auto &&el)
                                {
                                    acc.emplace_back(el.first, visit(*el.second).result());
                                    return acc;
                                }
                        )
                ));
            }
    );
}

inline std::unique_ptr<ast::Node>
replicator::result(void)
{
    return std::move(result_);
}

} // namespace kaleidoscope::ast::utils
