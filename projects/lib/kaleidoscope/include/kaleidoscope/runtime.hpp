#pragma once

#include <kaleidoscope/module.hpp>
#include <kaleidoscope/ast.hpp>

#include <kaleidoscope/ast/visitor.hpp>
#include <numeric>

#include <any>
#include <optional>
#include <list>

namespace kaleidoscope
{

struct runtime
{
    struct eval_node;     // forward decl
    struct ast_promotion; // forward decl

    struct result
    {
        result(void) = default;

        template<typename T>
            requires(!std::is_same_v<std::decay_t<T>, result>)
        result(T &&init)
            : storage_(std::forward<T>(init))
        {
        }

      public:

        result(const result &)            = default;
        result(result &&)                 = default;
        result &operator=(const result &) = default;
        result &operator=(result &&)      = default;

      public:

        template<typename T>
        operator T *(void)
        {
            return get_if<T>();
        }

        template<typename T>
            requires(!std::is_same_v<std::decay_t<T>, bool> && !std::is_pointer_v<T>)
        operator T(void)
        {
            return as<T>();
        }

        template<typename T>
        T
        as(void)
        {
            return std::any_cast<T>(storage_);
        }

        template<typename T>
        T *
        get_if(void)
        {
            return std::any_cast<T>(&storage_);
        }

        bool
        has_value(void) const
        {
            return storage_.has_value();
        }

        std::optional<std::type_index>
        type_index(void) const
        {
            if (storage_.has_value())
            {
                return storage_.type();
            }
            else
            {
                return std::nullopt;
            }
        }

        template<typename T>
        result &
        operator=(T &&value)
        {
            storage_ = std::forward<T>(value);
            return *this;
        }

        template<typename T>
        bool operator==(T &&rhs);

        operator bool(void) const
        {
            return storage_.has_value();
        }

      protected:

        std::any storage_;
    };

    using Scope = Module;

    runtime(void);
    runtime(Scope &global_scope);

    runtime(const runtime &copy_from);
    runtime(runtime &&move_from);
    runtime &operator=(const runtime &copy_from);
    runtime &operator=(runtime &&move_from);

    result eval(Module &module, const Ast &ast);

    runtime &push_scope();
    runtime &pop_scope();

  public:

    Scope &
    scope()
    {
        return *current_;
    }

    ast_promotion &
    get_ast_promotion(void)
    {
        return *ast_promotion_;
    }

  protected:

    std::unique_ptr<eval_node>                   eval_;
    std::unique_ptr<ast_promotion>               ast_promotion_;
    std::optional<std::reference_wrapper<Scope>> global_scope_;
    std::list<Scope>                             local_scopes_;
    Scope                                       *current_ = nullptr;
};

struct runtime::eval_node : public ast::utils::Visitor_Node_CRTP<eval_node, ast::Node>
{
    eval_node(runtime &owner);

    runtime::result eval(const Ast &ast);
    runtime::result eval(const ast::Node &node);

  protected:

    void visit_(const ast::Literal_Numeric &node);
    void visit_(const ast::Literal_String &node);
    void visit_(const ast::Variable &node);
    void visit_(const ast::Operation_Postfix &node);
    void visit_(const ast::Operation_Prefix &node);
    void visit_(const ast::Operation_Infix &node);
    void visit_(const ast::Functional_Call &node);

  protected:

    runtime::result result_;

    std::reference_wrapper<runtime> owner_;
};

struct runtime::ast_promotion : public ast::utils::Visitor_Node_CRTP<ast_promotion, ast::Node>
{
    ast_promotion(runtime &owner);

    runtime::result promote(const Ast &ast);
    runtime::result promote(const ast::Node &node);

  protected:

    void visit_(const ast::Literal_Numeric &node);
    void visit_(const ast::Literal_String &node);

  protected:

    runtime::result result_;

    std::reference_wrapper<runtime> owner_;
};

} // namespace kaleidoscope

namespace kaleidoscope
{

inline runtime::eval_node::eval_node(runtime &owner)
    : owner_(owner)
{
    register_method_handler<ast::Literal_Numeric>(
            static_cast<void (runtime::eval_node::*)(const ast::Literal_Numeric &)>(&runtime::eval_node::visit_)
    );

    register_method_handler<ast::Literal_String>(
            static_cast<void (runtime::eval_node::*)(const ast::Literal_String &)>(&runtime::eval_node::visit_)
    );

    register_method_handler<ast::Variable>(
            static_cast<void (runtime::eval_node::*)(const ast::Variable &)>(&runtime::eval_node::visit_)
    );

    register_method_handler<ast::Functional_Call>(
            static_cast<void (runtime::eval_node::*)(const ast::Functional_Call &)>(&runtime::eval_node::visit_)
    );

    register_method_handler<ast::Operation_Infix>(
            static_cast<void (runtime::eval_node::*)(const ast::Operation_Infix &)>(&runtime::eval_node::visit_)
    );

    register_method_handler<ast::Operation_Postfix>(
            static_cast<void (runtime::eval_node::*)(const ast::Operation_Postfix &)>(&runtime::eval_node::visit_)
    );

    register_method_handler<ast::Operation_Prefix>(
            static_cast<void (runtime::eval_node::*)(const ast::Operation_Prefix &)>(&runtime::eval_node::visit_)
    );
}

inline runtime::result
runtime::eval_node::eval(const Ast &ast)
{
    for (auto &&stmt : ast.statements)
    {
        result_ = eval(*stmt);
    }
    return result_;
}

inline runtime::result
runtime::eval_node::eval(const ast::Node &node)
{
    visit(node);
    return result_;
}

inline void
runtime::eval_node::visit_(const ast::Literal_Numeric &node)
{
    auto result = owner_.get().get_ast_promotion().promote(node);
    if (result)
    {
        result_ = std::move(result);
    }
    else
    {
        throw std::runtime_error("unknown value materialization from literal number");
    }
}

inline void
runtime::eval_node::visit_(const ast::Literal_String &node)
{
    auto result = owner_.get().get_ast_promotion().promote(node);
    if (result)
    {
        result_ = std::move(result);
    }
    else
    {
        throw std::runtime_error("unknown value materialization from literal string");
    }
}

inline void
runtime::eval_node::visit_(const ast::Variable &node)
{
    using namespace std::string_literals;

    auto func = owner_.get().scope().get_func(node.name);
    if (!func)
    {
        throw std::runtime_error("unknown variable: "s + node.name);
    }
    result_ = std::any_cast<double>(func({}));
}

inline void
runtime::eval_node::visit_(const ast::Functional_Call &node)
{
    using namespace std::string_literals;

    auto func = owner_.get().scope().get_func(node.callee);
    if (!func)
    {
        throw std::runtime_error("unknown function name: "s + node.callee);
    }

    auto args = std::accumulate(
            node.args.begin(),
            node.args.end(),
            std::vector<std::any> {},
            [&](auto acc, auto &&elem)
            {
                acc.emplace_back(eval(*elem));
                return acc;
            }
    );

    result_ = std::any_cast<double>(func(std::move(args)));
}

inline void
runtime::eval_node::visit_(const ast::Operation_Infix &node)
{
    using namespace std::string_literals;

    auto func = owner_.get().scope().get_binop(node.op);
    if (!func)
    {
        throw std::runtime_error("unknown binary operator: "s + node.op);
    }

    std::vector<std::any> args;
    args.emplace_back(eval(*node.lhs));
    args.emplace_back(eval(*node.rhs));

    result_ = std::any_cast<double>(func(std::move(args)));
}

inline void
runtime::eval_node::visit_(const ast::Operation_Postfix &node)
{
    using namespace std::string_literals;

    auto func = owner_.get().scope().get_unop(node.op);
    if (!func)
    {
        throw std::runtime_error("unknown unary operator: "s + node.op);
    }

    std::vector<std::any> args;
    args.emplace_back(eval(*node.operand));

    result_ = std::any_cast<double>(func(std::move(args)));
}

inline void
runtime::eval_node::visit_(const ast::Operation_Prefix &node)
{
    using namespace std::string_literals;

    auto func = owner_.get().scope().get_unop(node.op);
    if (!func)
    {
        throw std::runtime_error("unknown unary operator: "s + node.op);
    }

    std::vector<std::any> args;
    args.emplace_back(eval(*node.operand));

    result_ = std::any_cast<double>(func(std::move(args)));
}

} // namespace kaleidoscope

namespace kaleidoscope
{
inline runtime::runtime(void)
    : eval_(std::make_unique<eval_node>(*this))
{
    local_scopes_.push_back(Module {});
    current_ = &local_scopes_.back();
}

inline runtime::runtime(Scope &global_scope)
    : eval_(std::make_unique<eval_node>(*this))
    , ast_promotion_(std::make_unique<ast_promotion>(*this))
    , global_scope_(global_scope)
{
    if (global_scope_.has_value())
    {
        current_ = &global_scope_.value().get();
    }
    else
    {
        local_scopes_.push_back(Module {});
        current_ = &local_scopes_.back();
    }
}

inline runtime::runtime(const runtime &copy_from)
    : eval_(std::make_unique<eval_node>(*this))
    , ast_promotion_(std::make_unique<ast_promotion>(*this))
    , global_scope_(copy_from.global_scope_)
    , local_scopes_(copy_from.local_scopes_)
    , current_(copy_from.current_)
{
}

inline runtime::runtime(runtime &&move_from)
    : eval_(std::make_unique<eval_node>(*this))
    , ast_promotion_(std::make_unique<ast_promotion>(*this))
    , global_scope_(std::move(move_from.global_scope_))
    , local_scopes_(std::move(move_from.local_scopes_))
    , current_(std::move(move_from.current_))
{
}

inline runtime &
runtime::operator=(const runtime &copy_from)
{
    this->eval_          = std::move(std::make_unique<eval_node>(*this));
    this->ast_promotion_ = std::move(std::make_unique<ast_promotion>(*this));
    this->global_scope_  = copy_from.global_scope_;
    this->local_scopes_  = copy_from.local_scopes_;
    this->current_       = copy_from.current_;
    return *this;
}

inline runtime &
runtime::operator=(runtime &&move_from)
{
    this->eval_          = std::move(std::make_unique<eval_node>(*this));
    this->ast_promotion_ = std::move(std::make_unique<ast_promotion>(*this));
    std::swap(this->global_scope_, move_from.global_scope_);
    std::swap(this->local_scopes_, move_from.local_scopes_);
    std::swap(this->current_, move_from.current_);
    return *this;
}

inline runtime::result
runtime::eval(Module &module, const Ast &ast)
{
    return eval_->eval(ast);
}

inline runtime &
runtime::push_scope(void)
{
    auto &prev = *current_;
    local_scopes_.push_back(Module {});
    local_scopes_.back().link_shared(prev);
    current_ = &local_scopes_.back();

    return *this;
}

inline runtime &
runtime::pop_scope(void)
{
    if (local_scopes_.size() >= 2)
    {
        local_scopes_.pop_back();
        current_ = &local_scopes_.back();
    }
    else if (global_scope_.has_value())
    {
        local_scopes_.pop_back();
        current_ = &global_scope_.value().get();
    }
    else
    {
        current_ = &local_scopes_.back(); // родительских нет а эта последняя
    }
    return *this;
}

} // namespace kaleidoscope

namespace kaleidoscope
{

inline runtime::ast_promotion::ast_promotion(runtime &owner)
    : owner_(owner)
{
    register_method_handler<ast::Literal_Numeric>(
            static_cast<void (runtime::ast_promotion::*)(const ast::Literal_Numeric &)>(&runtime::ast_promotion::visit_)
    );

    register_method_handler<ast::Literal_String>(
            static_cast<void (runtime::ast_promotion::*)(const ast::Literal_String &)>(&runtime::ast_promotion::visit_)
    );
}

inline runtime::result
runtime::ast_promotion::promote(const Ast &ast)
{
    for (auto &&stmt : ast.statements)
    {
        result_ = promote(*stmt);
    }
    return result_;
}

inline runtime::result
runtime::ast_promotion::promote(const ast::Node &node)
{
    visit(node);
    return result_;
}

inline void
runtime::ast_promotion::visit_(const ast::Literal_Numeric &node)
{
    result_ = std::visit([](const auto &value) -> runtime::result
                         {
                             return value;
                         },
                         node.value);
}

inline void
runtime::ast_promotion::visit_(const ast::Literal_String &node)
{
    result_ = node.value;
}

template<typename T>
bool
runtime::result::operator==(T &&rhs)
{
    using Value = std::decay_t<T>;
    Value *lhs  = *this;
    return (lhs) ? *lhs == std::forward<T>(rhs) : false;
}

} // namespace kaleidoscope
