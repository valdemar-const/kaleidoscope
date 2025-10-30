#pragma once

#include <kaleidoscope/ast.hpp>
#include <kaleidoscope/ast/visitor.hpp>

#include <list>
#include <vector>
#include <iterator>
#include <concepts>
#include <type_traits>

namespace kaleidoscope::ast::utils
{

template<traits::Ast_Node T>
std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>
each(std::unique_ptr<T> &node)
{
    return {};
}

} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{

template<>
inline std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>
each<ast::Function_Defenition>(std::unique_ptr<ast::Function_Defenition> &node)
{
    using Result = std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>;

    auto stmts = std::accumulate(
            node->body.begin(),
            node->body.end(),
            Result {},
            [](auto acc, auto &&el)
            {
                acc.emplace_back(std::ref(el));
                return acc;
            }
    );

    Result result = {std::ref(*reinterpret_cast<std::unique_ptr<ast::Node> *>(&node->prototype))};
    result.insert(result.end(), stmts.begin(), stmts.end());
    return result;
}

template<>
inline std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>
each<ast::Functional_Call>(std::unique_ptr<ast::Functional_Call> &node)
{
    using Result = std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>;

    return std::accumulate(
            node->args.begin(),
            node->args.end(),
            Result {},
            [](auto acc, auto &&el)
            {
                acc.emplace_back(std::ref(el));
                return acc;
            }
    );
}

template<>
inline std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>
each<ast::Operation_Infix>(std::unique_ptr<ast::Operation_Infix> &node)
{
    return {std::ref(node->lhs), std::ref(node->rhs)};
}

template<>
inline std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>
each<ast::Operation_Unary>(std::unique_ptr<ast::Operation_Unary> &node)
{
    return {std::ref(node->operand)};
}

template<>
inline std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>
each<ast::Precedence_Agnostic_Expr>(std::unique_ptr<ast::Precedence_Agnostic_Expr> &node)
{
    using Result = std::vector<std::reference_wrapper<std::unique_ptr<ast::Node>>>;
    Result result {std::ref(node->first)};

    auto args = std::accumulate(
            node->operations.begin(),
            node->operations.end(),
            Result {},
            [](auto acc, auto &&el)
            {
                acc.emplace_back(std::ref(el.second));
                return acc;
            }
    );
    result.insert(result.end(), std::make_move_iterator(args.begin()), std::make_move_iterator(args.end()));

    return result;
}

} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{

struct Iterator_Recursive
{
    using iterator_concept = std::forward_iterator_tag;
    using element_type     = ast::INodeRef;
    using pointer_type     = ast::INodePtr;
    using reference_type   = ast::INodeRef;
    using Parents          = std::list<element_type>;

    struct Next;

    Iterator_Recursive(ast::Node &root, Parents parents = {})
        : current_(root)
        , parents_(std::move(parents))
        , next(std::make_unique<Next>())
    {
    }

    reference_type operator*(void) const;
    pointer_type   operator->(void) const;

    Iterator_Recursive operator++(void);

  protected:

    element_type            current_;
    std::list<element_type> parents_;
    std::unique_ptr<Next>   next;
};

struct Iterator_Recursive::Next : public ast::utils::Visitor_Node_CRTP<Iterator_Recursive::Next, ast::Node>
{
    Next(void)
    {
    }
};

} // namespace kaleidoscope::ast::utils
