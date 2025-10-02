#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>

#include <concepts>
#include <type_traits>
#include <typeindex>

#include <anyany/anyany.hpp>
#include <anyany/type_descriptor.hpp>

#include <cinttypes>

#include <compiler/demangle.hpp>
#include <cassert>

namespace aa
{

struct type_info_rtti
{
    template<typename T>
    static aa::descriptor_t
    do_invoke(const T &self)
    {
        if constexpr (std::is_polymorphic_v<std::decay_t<decltype(self)>>)
        {
            static const auto typename_ = compiler::demangle(typeid(self).name()) + "]";
            return aa::descriptor_t {typename_.data()};
        }
        else
        {
            return aa::descriptor_v<std::decay_t<decltype(self)>>;
        }
    }

    template<typename CRTP>
    struct plugin
    {
        aa::descriptor_t
        type_descriptor() const
        {
            return aa::invoke<::aa::type_info_rtti>(static_cast<const CRTP &>(*this));
        }
    };
};

struct type_index
{
    template<typename T>
    static std::type_index
    do_invoke(const T &self)
    {
        return std::type_index(typeid(self));
    }

    template<typename CRTP>
    struct plugin
    {
        std::type_index
        type_index() const
        {
            return aa::invoke<::aa::type_index>(static_cast<const CRTP &>(*this));
        }
    };
};

/** any_with included all type information for both RTTI type_index and anyany descriptor_t */
template<typename... Args>
using any_with_t = aa::any_with<type_info_rtti, type_index, Args...>;

/** poly_ref included all type information for both RTTI type_index and anyany descriptor_t */
template<typename... Args>
using poly_ref_t = aa::poly_ref<type_info_rtti, type_index, Args...>;

/** poly_ptr included all type information for both RTTI type_index and anyany descriptor_t */
template<typename... Args>
using poly_ptr_t = aa::poly_ptr<type_info_rtti, type_index, Args...>;

} // namespace aa

namespace kaleidoscope::ast
{
using INode    = aa::any_with_t<aa::move>;
using INodeRef = aa::poly_ref_t<>;
} // namespace kaleidoscope::ast

namespace kaleidoscope::ast
{

struct Lexeme_Numeric
{
    using f64   = double;
    using Value = std::variant<f64>;

    Lexeme_Numeric(Value value)
        : value(value)
    {
    }

    Lexeme_Numeric(Lexeme_Numeric &&)            = default;
    Lexeme_Numeric &operator=(Lexeme_Numeric &&) = default;

    Value value;
};

struct Variable
{
    using Name = std::string;

    Variable(Name name)
        : name(name)
    {
    }

    Variable(Variable &&)            = default;
    Variable &operator=(Variable &&) = default;

    operator std::string(void)
    {
        return name;
    };

    Name name;
};

struct Function_Declaration
{
    using Name = std::string;
    using Arg  = std::string;
    using Args = std::vector<Arg>;

    Function_Declaration(Name name, Args args)
        : name(name)
        , args(args)
    {
    }

    Function_Declaration(Function_Declaration &&)            = default;
    Function_Declaration &operator=(Function_Declaration &&) = default;

    Name name;
    Args args;
};

struct Function_Defenition
{
    using Prototype = std::unique_ptr<Function_Declaration>;
    using Statement = ast::INode;
    using Body      = std::vector<Statement>;

    Function_Defenition(Prototype prototype, Body body)
        : prototype(std::move(prototype))
        , body(std::move(body))
    {
    }

    Function_Defenition(Function_Defenition &&)            = default;
    Function_Defenition &operator=(Function_Defenition &&) = default;

    Prototype prototype;
    Body      body;
};

struct Functional_Call
{
    using Callee = std::string;
    using Args   = std::vector<ast::INode>;

    Functional_Call(Callee callee, Args args)
        : callee(callee)
        , args(std::move(args))
    {
    }

    Functional_Call(Functional_Call &&)            = default;
    Functional_Call &operator=(Functional_Call &&) = default;

    Callee callee;
    Args   args;
};

struct Operation_Unary
{
    using Operator   = std::string;
    using Expression = ast::INode;

    Operation_Unary(Operator op, Expression operand)
        : op(op)
        , operand(std::move(operand))
    {
    }

    Operation_Unary(Operation_Unary &&)            = default;
    Operation_Unary &operator=(Operation_Unary &&) = default;

    Operator   op;
    Expression operand;
};

struct Operation_Binary
{
    using Operator   = std::string;
    using Expression = ast::INode;

    Operation_Binary(Operator op, Expression lhs, Expression rhs)
        : op(op)
        , lhs(std::move(lhs))
        , rhs(std::move(rhs))
    {
    }

    Operation_Binary(Operation_Binary &&)            = default;
    Operation_Binary &operator=(Operation_Binary &&) = default;

    Operator   op;
    Expression lhs;
    Expression rhs;
};

struct Precedence_Agnostic_Expr
{
    using Expression = ast::INode;
    using Op         = std::string;
    using Operations = std::vector<std::pair<Op, Expression>>;

    Precedence_Agnostic_Expr(Expression first, Operations operations)
        : first(std::move(first))
        , operations(std::move(operations))
    {
    }

    Precedence_Agnostic_Expr(Precedence_Agnostic_Expr &&)            = default;
    Precedence_Agnostic_Expr &operator=(Precedence_Agnostic_Expr &&) = default;

    Expression first;
    Operations operations;
};

// TODO: to implement
struct If_Expression
{
    using Condition    = ast::INode;
    using Expression   = ast::INode;
    using Truly_Result = Expression;
    using Falsy_Result = Expression;

    If_Expression(Condition condition, Truly_Result truly, Falsy_Result falsy)
        : condition(std::move(condition))
        , truly(std::move(truly))
        , falsy(std::move(falsy))
    {
    }

    If_Expression(If_Expression &&)            = default;
    If_Expression &operator=(If_Expression &&) = default;

    Expression   condition;
    Truly_Result truly;
    Falsy_Result falsy;
};

} // namespace kaleidoscope::ast

namespace kaleidoscope
{

struct Ast
{
    using Statements = std::vector<ast::INode>;

    ~Ast(void) = default;
    Ast(void)  = default;

    Ast(Ast &)  = delete;
    Ast(Ast &&) = default;

    Ast &operator=(Ast &)  = delete;
    Ast &operator=(Ast &&) = default;

    Ast(Statements statements)
        : statements(std::move(statements))
    {
    }

    Statements statements;
};

} // namespace kaleidoscope
