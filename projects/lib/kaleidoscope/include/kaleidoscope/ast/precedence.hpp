#pragma once

#include <kaleidoscope/module.hpp>
#include <kaleidoscope/ast/visitor.hpp>
#include <kaleidoscope/ast/replicator.hpp>

#include <ranges>
#include <functional>
#include <list>

namespace kaleidoscope::ast::utils
{

/**
 * @brief Спуститься по AST и найти все Precedence_Agnostic_Expr
 *        и заменить их все узлами выражений согласно приоритету операторов
 *
 */
struct precedence : ast::utils::Visitor_Node_CRTP<precedence, kaleidoscope::ast::Node>
{
    using Super              = ast::utils::Visitor_Node_CRTP<precedence, kaleidoscope::ast::Node>;
    using Bin_Op_Precedence  = Module::precedence;
    using Expression_Element = std::variant<ast::Node *, std::string>;
    using Output             = std::vector<Expression_Element>;
    using Result             = std::list<ast::Precedence_Agnostic_Expr::Expression>;

  public: // api

    precedence(const Bin_Op_Precedence &precedence);
    ~precedence(void) = default;

    void operator()(Ast &ast);

    std::unique_ptr<ast::Node> result(void);

  protected:

    void visit_(const ast::Operation_Unary &ast);
    void visit_(const ast::Function_Defenition &ast);
    void visit_(const ast::Functional_Call &ast);
    void visit_(const ast::Precedence_Agnostic_Expr &ast);

  private:

    bool
    pop_to_result(Output &from);

  protected:

    std::reference_wrapper<const Bin_Op_Precedence> precedence_;
    Result                                          converted_;
};

} // namespace kaleidoscope::ast::utils

namespace kaleidoscope::ast::utils
{

inline precedence::precedence(const Bin_Op_Precedence &precedence)
    : Super(true)
    , precedence_(precedence)
{
    using namespace std::placeholders;

    register_method_handler<ast::Operation_Unary>(
            static_cast<void (precedence::*)(const ast::Operation_Unary &)>(&precedence::visit_)
    );
    register_method_handler<ast::Function_Defenition>(
            static_cast<void (precedence::*)(const ast::Function_Defenition &)>(&precedence::visit_)
    );
    register_method_handler<ast::Functional_Call>(
            static_cast<void (precedence::*)(const ast::Functional_Call &)>(&precedence::visit_)
    );
    register_method_handler<ast::Precedence_Agnostic_Expr>(
            static_cast<void (precedence::*)(const ast::Precedence_Agnostic_Expr &)>(&precedence::visit_)
    );
}

inline void
precedence::operator()(Ast &ast)
{
    static const auto is_expr_node = [](auto &&elem) -> bool
    {
        return typeid(ast::Precedence_Agnostic_Expr) == typeid(*elem);
    };

    auto &prec_agnosics = ast.statements; // | std::ranges::views::filter(is_expr_node);
    for (auto &&stmt : prec_agnosics)
    {
        visit(*static_cast<ast::Precedence_Agnostic_Expr *>(stmt.get()));

        if (!converted_.empty())
        {
            stmt.reset(converted_.back().release());
            converted_.pop_back();
        }
    }
}

inline std::unique_ptr<ast::Node>
precedence::result(void)
{
    if (converted_.empty())
    {
        return {};
    }
    else
    {
        std::unique_ptr<ast::Node> tmp {converted_.back().release()};
        converted_.pop_back();
        return std::move(tmp);
    }
}

inline void
precedence::visit_(const ast::Operation_Unary &ast)
{
    if (typeid(*ast.operand) == typeid(ast::Precedence_Agnostic_Expr))
    {
        const_cast<ast::Operation_Unary::Expression &>(ast.operand).reset(visit(*ast.operand).result().release());
    }
    else
    {
        // do nothing
    }
    visit(*ast.operand);
}

inline void
precedence::visit_(const ast::Function_Defenition &ast)
{
    for (auto &&stmt : ast.body)
    {
        if (typeid(*stmt) == typeid(ast::Precedence_Agnostic_Expr))
        {
            const_cast<ast::Function_Defenition::Statement &>(stmt).reset(visit(*stmt).result().release());
        }
        else
        {
            // do nothing
        }
        visit(*stmt);
    }
}

inline void
precedence::visit_(const ast::Functional_Call &ast)
{
    for (auto &&arg : ast.args)
    {
        if (typeid(*arg) == typeid(ast::Precedence_Agnostic_Expr))
        {
            const_cast<ast::Precedence_Agnostic_Expr::Expression &>(arg).reset(visit(*arg).result().release());
        }
        else
        {
            visit(*arg);
        }
    }
}

inline void
precedence::visit_(const ast::Precedence_Agnostic_Expr &ast)
{
    auto &input = ast.operations;
    // Reverse Polish Notation
    if (typeid(*ast.first) == typeid(ast::Precedence_Agnostic_Expr))
    {
        const_cast<ast::Precedence_Agnostic_Expr::Expression &>(ast.first).reset(visit(*ast.first).result().release());
    }
    else
    {
        visit(*ast.first);
    }

    Output                 output {ast.first.get()};
    std::list<std::string> ops;

    for (auto &&op : input)
    {
        if (!precedence_.get().contains(op.first))
        {
            throw std::runtime_error("Unknown operator precedence: " + op.first);
        }

        if (typeid(*op.second) == typeid(ast::Precedence_Agnostic_Expr))
        {
            const_cast<ast::Precedence_Agnostic_Expr::Expression &>(op.second).reset(visit(*op.second).result().release());
        }
        else
        {
            visit(*op.second);
        }

        auto &curr_info = precedence_.get().at(op.first);

        if (!ops.empty())
        {
            auto &top_info = precedence_.get().at(ops.back());
            if (top_info.precedence < curr_info.precedence)
            {
                output.emplace_back(ops.back());
                ops.pop_back();
                ops.emplace_back(op.first);
                output.emplace_back(op.second.get());
            }
            else
            {
                ops.emplace_back(op.first);
                output.emplace_back(op.second.get());
            }
        }
        else
        {
            ops.emplace_back(op.first);
            output.emplace_back(op.second.get());
        }
    }

    std::copy(ops.rbegin(), ops.rend(), std::back_insert_iterator(output));
    ops.clear();

    // build Expression Priority Ast
    auto converted = pop_to_result(output);
}

inline bool
precedence::pop_to_result(Output &from)
{
    if (from.empty())
    {
        return false;
    }

    bool result = true;

    auto top = from.back();

    if (auto op = std::get_if<std::string>(&top))
    {
        from.pop_back();
        pop_to_result(from); // rhs
        pop_to_result(from); // lhs

        std::unique_ptr<ast::Node> lhs {converted_.back().release()};
        converted_.pop_back();

        std::unique_ptr<ast::Node> rhs {converted_.back().release()};
        converted_.pop_back();

        converted_.emplace_back(std::make_unique<ast::Operation_Binary>(*op, std::move(lhs), std::move(rhs)));
    }
    else if (auto operand = *std::get_if<ast::Node *>(&top))
    {
        from.pop_back();
        converted_.emplace_back(utils::replicator {}.visit(*operand).result());
    }
    else
    {
        result = false;
    }

    return result;
}

} // namespace kaleidoscope::ast::utils
