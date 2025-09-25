#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <cinttypes>

#include <concepts>
#include <type_traits>

namespace kaleidoscope::ast
{
struct Node; // forward decl
} // namespace kaleidoscope::ast

namespace kaleidoscope::traits
{

template<typename T>
concept Ast_Node = requires {
    requires std::is_base_of_v<ast::Node, T>;
};

} // namespace kaleidoscope::traits

namespace kaleidoscope::ast
{
struct Node
{
    virtual ~Node(void) = default;
};

template<typename T>
struct NodeCRTP : public Node
{
    ~NodeCRTP(void) override = default;
};

struct Lexeme_Numeric : public NodeCRTP<Lexeme_Numeric>
{
    using f64   = double;
    using Value = std::variant<f64>;

    ~Lexeme_Numeric(void) override = default;

    Lexeme_Numeric(Value value)
        : value(value)
    {
    }

    Value value;
};

struct Variable : public NodeCRTP<Variable>
{
    using Name               = std::string;
    ~Variable(void) override = default;

    Variable(Name name)
        : name(name)
    {
    }

    operator std::string(void)
    {
        return name;
    };

    Name name;
};

struct Function_Declaration : public NodeCRTP<Function_Declaration>
{
    using Name = std::string;
    using Arg  = std::string;
    using Args = std::vector<Arg>;

    ~Function_Declaration(void) override = default;

    Function_Declaration(Name name, Args args)
        : name(name)
        , args(args)
    {
    }

    Name name;
    Args args;
};

struct Function_Defenition : public NodeCRTP<Function_Defenition>
{
    using Prototype = std::unique_ptr<Function_Declaration>;
    using Statement = std::unique_ptr<Node>;
    using Body      = std::vector<Statement>;

    ~Function_Defenition(void) override = default;

    Function_Defenition(Prototype prototype, Body body)
        : prototype(std::move(prototype))
        , body(std::move(body))
    {
    }

    Prototype prototype;
    Body      body;
};

struct Functional_Call : public NodeCRTP<Functional_Call>
{
    using Callee = std::string;
    using Args   = std::vector<std::unique_ptr<Node>>;

    ~Functional_Call(void) override = default;

    Functional_Call(Callee callee, Args args)
        : callee(callee)
        , args(std::move(args))
    {
    }

    Callee callee;
    Args   args;
};

struct Operation_Unary : public NodeCRTP<Operation_Unary>
{
    using Operator   = std::string;
    using Expression = std::unique_ptr<Node>;

    ~Operation_Unary(void) override = default;

    Operation_Unary(Operator op, Expression operand)
        : op(op)
        , operand(std::move(operand))
    {
    }

    Operator   op;
    Expression operand;
};

struct Operation_Binary : public NodeCRTP<Operation_Binary>
{
    using Operator   = std::string;
    using Expression = std::unique_ptr<Node>;

    ~Operation_Binary(void) override = default;

    Operation_Binary(Operator op, Expression lhs, Expression rhs)
        : op(op)
        , lhs(std::move(lhs))
        , rhs(std::move(rhs))
    {
    }

    Operator   op;
    Expression lhs;
    Expression rhs;
};

struct Precedence_Agnostic_Expr : public NodeCRTP<Precedence_Agnostic_Expr>
{
    using Expression = std::unique_ptr<Node>;
    using Op         = std::string;
    using Operations = std::vector<std::pair<Op, Expression>>;

    ~Precedence_Agnostic_Expr(void) override = default;

    Precedence_Agnostic_Expr(Expression first, Operations operations)
        : first(std::move(first))
        , operations(std::move(operations))
    {
    }

    Expression first;
    Operations operations;
};

// TODO: to implement
struct If_Expression : public NodeCRTP<If_Expression>
{
    using Condition    = std::unique_ptr<Node>;
    using Expression   = std::unique_ptr<Node>;
    using Truly_Result = Expression;
    using Falsy_Result = Expression;

    If_Expression(Condition condition, Truly_Result truly, Falsy_Result falsy)
        : condition(std::move(condition))
        , truly(std::move(truly))
        , falsy(std::move(falsy))
    {
    }

    Expression   condition;
    Truly_Result truly;
    Falsy_Result falsy;
};

} // namespace kaleidoscope::ast

namespace kaleidoscope
{

struct Ast
{
    using Statements = std::vector<std::unique_ptr<ast::Node>>;

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
