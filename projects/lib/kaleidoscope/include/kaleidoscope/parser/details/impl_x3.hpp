#pragma once

#include <kaleidoscope/parser/details/ast_fusion_adapt.hpp>
#include <type_traits>

#include <numeric>
#include <iostream>
#include <variant>
#include <type_traits>

#include <boost/spirit/home/x3.hpp>

namespace kaleidoscope::parser
{
struct mutable_tag;

struct var_decl_attr
{
    bool is_mutable = false;
};

} // namespace kaleidoscope::parser

namespace kaleidoscope::parser::actions
{
using boost::fusion::at_c;

const auto emplace_to_vec = [](auto &ctx)
{
    _val(ctx).emplace_back(_attr(ctx).release());
};

const auto node_upcast = [](auto &ctx)
{
    _val(ctx).reset(_attr(ctx).release());
};

const auto vector_node_upcast = [](auto &ctx)
{
    auto &attr   = _attr(ctx);
    auto &result = _val(ctx);

    for (auto &elem : attr)
    {
        result.emplace_back(elem.release());
    }
};

const auto variant_node_upcast = [](auto &ctx)
{
    boost::apply_visitor(
            [&](auto &&v)
            {
                _val(ctx).reset(v.release());
            },
            _attr(ctx)
    );
};

const auto number_lit_parsed = [](auto &ctx)
{
    using AttrZero            = char;
    using AttrFirstNum        = char;
    using AttrDot             = char;
    using AttrIntegralNumbers = std::string;
    using AttrFloatingPart    = boost::fusion::deque<AttrDot, std::string>;
    using AttrNumeric         = boost::fusion::deque<AttrFirstNum, AttrIntegralNumbers, boost::optional<AttrFloatingPart>>;
    using Attr                = boost::variant<AttrZero, AttrNumeric>;

    auto &numeric = _attr(ctx);
    static_assert(std::is_same_v<std::decay_t<decltype(numeric)>, Attr>, "bad attr");

    if (numeric.which() == 0)
    {
        _val(ctx) = std::make_unique<ast::Literal_Numeric>(0LL);
    }
    else if (auto parsed_ptr = boost::get<AttrNumeric>(&numeric))
    {
        auto &parsed = *parsed_ptr;

        auto [first_digit, other_digits, float_part] = std::make_tuple(at_c<0>(parsed), at_c<1>(parsed), at_c<2>(parsed));

        std::string number_str = std::string(1, first_digit) + other_digits;

        if (float_part.has_value())
        {
            auto [dot, float_digits]  = std::make_tuple(at_c<0>(*float_part), at_c<1>(*float_part));
            number_str               += dot + float_digits;
            double value              = std::stod(number_str);
            _val(ctx)                 = std::make_unique<ast::Literal_Numeric>(value);
        }
        else
        {
            int64_t value = std::stoll(number_str);
            _val(ctx)     = std::make_unique<ast::Literal_Numeric>(value);
        }
    }
};

const auto number_parsed = [](auto &ctx)
{
    boost::apply_visitor(
            [&](const auto &number)
            {
                _val(ctx).reset(new ast::Literal_Numeric(ast::Literal_Numeric::Value {number}));
            },
            _attr(ctx)
    );
};

const auto string_parsed = [](auto &ctx)
{
    _val(ctx).reset(new ast::Literal_String(std::move(_attr(ctx))));
};

const auto variable_parsed = [](auto &ctx)
{
    _val(ctx).reset(new ast::Variable(_attr(ctx)));
};

const auto fun_decl_parsed = [](auto &ctx)
{
    _val(ctx).reset(new ast::Function_Declaration(at_c<0>(_attr(ctx)), at_c<1>(_attr(ctx))));
};

const auto fun_def_parsed = [](auto &ctx)
{
    auto &func_stmts = at_c<1>(_attr(ctx));
    auto  converted  = std::accumulate(
            func_stmts.begin(),
            func_stmts.end(),
            ast::Function_Defenition::Body {},
            [](auto acc, auto &&elem)
            {
                acc.emplace_back(elem.release());
                return acc;
            }
    );
    _val(ctx).reset(new ast::Function_Defenition(std::move(at_c<0>(_attr(ctx))), std::move(converted)));
};

const auto fun_call_parsed = [](auto &ctx)
{
    auto &callee   = at_c<0>(_attr(ctx));
    auto &arg_list = at_c<1>(_attr(ctx));
#if 1
    _val(ctx).reset(
            new ast::Functional_Call(
                    callee,
                    (arg_list.has_value()) ? std::move(*arg_list)
                                           : ast::Functional_Call::Args {}
            )
    );
#else
    _val(ctx).reset(new ast::Functional_Call(callee, std::move(arg_list)));
#endif
};

const auto postfix_expr_parsed = [](auto &ctx)
{
    auto &expr = at_c<0>(_attr(ctx));
    auto &op   = at_c<1>(_attr(ctx));

    if (op.has_value())
    {
        _val(ctx).reset(new ast::Operation_Postfix(*op, std::move(expr)));
    }
    else
    {
        _val(ctx).reset(expr.release());
    }
};

const auto prefix_expr_parsed = [](auto &ctx)
{
    auto &op   = at_c<0>(_attr(ctx));
    auto &expr = at_c<1>(_attr(ctx));

    if (op.has_value())
    {
        _val(ctx).reset(new ast::Operation_Prefix(*op, std::move(expr)));
    }
    else
    {
        _val(ctx).reset(expr.release());
    }
};

const auto expr_next_parsed = [](auto &ctx)
{
#if 0
    auto &ops     = at_c<0>(_attr(ctx));
    auto &operand = at_c<1>(_attr(ctx));
    if (ops.size() == 1)
    {
        _val(ctx) = std::move(std::make_pair(ops.front(), std::move(operand)));
    }
    else if (ops.size() == 2)
    {
        auto unary_expression =
                std::accumulate(
                        ops.rbegin(), ops.rend() - 1, std::unique_ptr<ast::Node> {operand.release()}, [](auto acc, auto &&op)
                        {
                            return std::unique_ptr<ast::Node>(new ast::Operation_Prefix {op, std::move(acc)});
                        }
                );
        _val(ctx) = std::move(std::make_pair(ops.front(), std::move(unary_expression)));
    }
    else
    {
        // do nothing
        _pass(ctx) = false;
    }
#else
    auto &op      = at_c<0>(_attr(ctx));
    auto &operand = at_c<1>(_attr(ctx));
    _val(ctx)     = std::move(std::make_pair(op, std::move(operand)));
#endif
};

const auto expr_parsed = [](auto &ctx)
{
#if 1
    auto &first      = at_c<0>(_attr(ctx));
    auto &operations = at_c<1>(_attr(ctx));

    _val(ctx).reset(new ast::Precedence_Agnostic_Expr(
            std::move(first),
            std::move(operations)
    ));
#else
    auto &maybe_op   = at_c<0>(_attr(ctx));
    auto &expr       = at_c<1>(_attr(ctx));
    auto &operations = at_c<2>(_attr(ctx));

    std::unique_ptr<ast::Node> first;

    if (maybe_op.has_value())
    {
        first.reset(new ast::Operation_Prefix {maybe_op.value(), std::move(expr)});
    }
    else
    {
        first = std::move(expr);
    }

    _val(ctx).reset(new ast::Precedence_Agnostic_Expr(
            std::move(first),
            std::move(operations)
    ));
#endif
};

const auto tail_parsed = [](auto &ctx)
{
    _val(ctx) = std::move(_attr(ctx));
};

const auto expr_list_parsed = [](auto &ctx)
{
    auto          &first  = at_c<0>(_attr(ctx));
    auto          &tail   = at_c<1>(_attr(ctx));
    volatile void *break_ = nullptr;

    std::vector<std::unique_ptr<ast::Node>> result;
    result.emplace_back(first.release());

    for (auto &expr : tail)
    {
        result.emplace_back(expr.release());
    }

    _val(ctx) = std::move(result);
};

const auto type_decl_parsed = [](auto &ctx)
{
    _val(ctx).reset(new ast::Type_Declaration(_attr(ctx)));
};

const auto make_mutable = [](auto &ctx)
{
    boost::spirit::x3::get<mutable_tag>(ctx).is_mutable = true;
};

const auto data_object_decl_parsed = [](auto &ctx)
{
    auto &attr = _attr(ctx); // fusion::deque<identifier_list, type_decl, optional<expr>>

    _val(ctx).reset(new ast::Data_Object_Definition_List(
            std::move(at_c<0>(attr)), // names
            static_cast<ast::Data_Object_Definition_List::Mutability>(
                    boost::spirit::x3::get<mutable_tag>(ctx).is_mutable
            ),
            std::move(at_c<1>(attr)), // type
            at_c<2>(attr) ? std::move(*at_c<2>(attr)) : nullptr
    ));
};

const auto chunk_parsed = [](auto &ctx)
{
    _val(ctx) = std::move(Ast(std::move(_attr(ctx))));
};

}; // namespace kaleidoscope::parser::actions

namespace kaleidoscope::parser::grammar
{
namespace x3 = boost::spirit::x3;
using namespace actions;

// clang-format off
const x3::rule<class R_Chunk,     std::vector< std::unique_ptr< ast::Node                       >>> chunk           = "chunk";
const x3::rule<class R_Stmt_List, std::vector< std::unique_ptr< ast::Node                       >>> stmt_list       = "statement-list";
const x3::rule<class R_Stmt,                   std::unique_ptr< ast::Node                        >> stmt            = "statement";
const x3::rule<class R_Type_Decl,              std::unique_ptr< ast::Type_Declaration            >> type_decl       = "type-declaration";
const x3::rule<class R_Var_Def,                std::unique_ptr< ast::Data_Object_Definition_List >> var_def         = "data-object-definition";
const x3::rule<class R_Fun_Decl,               std::unique_ptr< ast::Function_Declaration        >> fun_decl        = "function-declaration";
const x3::rule<class R_Fun_Block, std::vector< std::unique_ptr< ast::Node                       >>> fun_block       = "function-block";
const x3::rule<class R_Fun_Def,                std::unique_ptr< ast::Function_Defenition         >> fun_def         = "function-defenition";
const x3::rule<class R_Expr_list, std::vector< std::unique_ptr< ast::Node                       >>> expr_list       = "expression-list";
const x3::rule<class R_Expr,                   std::unique_ptr< ast::Precedence_Agnostic_Expr    >> expr            = "expression-raw";
const x3::rule<class R_Expr_Group,             std::unique_ptr< ast::Precedence_Agnostic_Expr    >> expr_grouped    = "expression-grouped";
const x3::rule<class R_Expr_Next,   std::pair<
                                               ast::Precedence_Agnostic_Expr::Op,
                                               std::unique_ptr< ast::Node >
                                             >                                                    > expr_next       = "expression-continue";
const x3::rule<class R_Atom,                   std::unique_ptr< ast::Node                        >> atom            = "atom";
const x3::rule<class R_Postfix_Expr,           std::unique_ptr< ast::Node                        >> postfix_expr    = "postfix-expression";
const x3::rule<class R_Prefix_Expr,            std::unique_ptr< ast::Node                        >> prefix_expr     = "prefix-expression";
const x3::rule<class R_Term,                   std::unique_ptr< ast::Node                        >> term            = "term";
const x3::rule<class R_Simple,                 std::unique_ptr< ast::Node                        >> simple          = "simple";
const x3::rule<class R_Fun_Call,               std::unique_ptr< ast::Functional_Call             >> fun_call        = "functional-call";
const x3::rule<class R_Var,                    std::unique_ptr< ast::Variable                    >> variable        = "variable";
const x3::rule<class R_Number,                 std::unique_ptr< ast::Literal_Numeric             >> number          = "number";
const x3::rule<class R_String,                 std::unique_ptr< ast::Literal_String              >> string          = "string";
const x3::rule<class R_Identifier_List,        std::vector    < std::string                      >> identifier_list = "identifier-list";
const x3::rule<class R_Identifier,                              std::string                       > identifier      = "identifier";

// clang-format on

template<typename S>
auto
mkkw(S &&kw)
{
    return x3::lexeme[x3::lit(std::string(kw)) >> !x3::alnum];
}

template<typename S>
auto
mkop(S &&op)
{
    return x3::lexeme[x3::lit(std::string(op)) >> !x3::alnum];
}

// keywords

const auto kw_module    = mkkw("module");    // module definition
const auto kw_export    = mkkw("export");    // export module section
const auto kw_doc       = mkkw("doc");       // documentation block
const auto kw_spec      = mkkw("spec");      // specification block
const auto kw_pub       = mkkw("pub");       // mark symbol public
const auto kw_implement = mkkw("implement"); // module/interface implementation
const auto kw_interface = mkkw("interface"); // interface for dynamic dispatch (type erasure)
const auto kw_import    = mkkw("import");    // module system
const auto kw_from      = mkkw("from");      // module system
const auto kw_type      = mkkw("type");      // type definition
const auto kw_enum      = mkkw("enum");      // tagged enumerators
const auto kw_callable  = mkkw("callable");  // callable semantic type
const auto kw_struct    = mkkw("struct");    // memory layout
const auto kw_tuple     = mkkw("tuple");     // memory layout
const auto kw_array     = mkkw("array");     // memory layout
const auto kw_vector    = mkkw("vector");    // managed array
const auto kw_any_of    = mkkw("any_of");    // tagged union memory layout
const auto kw_any_with  = mkkw("any_with");  // polymorphic value type (erased)
const auto kw_range     = mkkw("range");     // range semantic
const auto kw_addr      = mkkw("addr");      // get object address operator
const auto kw_optional  = mkkw("optional");  // optional value semantic
const auto kw_ref       = mkkw("ref");       // borrowed reference
const auto kw_ptr       = mkkw("ptr");       // pointer semantic
const auto kw_shared    = mkkw("shared");    // shared managed value
const auto kw_owned     = mkkw("owned");     // unique managed value
const auto kw_weak      = mkkw("weak");      // observable unmanaged value
const auto kw_any       = mkkw("any_with");  // type erased configurable value type
const auto kw_var       = mkkw("var");       // mutable stack value
const auto kw_let       = mkkw("let");       // stack value
const auto kw_mut       = mkkw("mut");       // allow value mutation
const auto kw_operator  = mkkw("operator");  // operator definition
const auto kw_literal   = mkkw("literal");   // custom lexeme suffixes
const auto kw_function  = mkkw("function");  // function definition
const auto kw_apply     = mkkw("apply");     // destructured binding to callable arguments
const auto kw_return    = mkkw("return");    // explicit return statement
const auto kw_block     = mkkw("block");     // operator composition label system
const auto kw_do        = mkkw("do");        // start code block
const auto kw_end       = mkkw("end");       // end code block
const auto kw_loop      = mkkw("loop");      // endless loop
const auto kw_repeat    = mkkw("repeat");    // loop with postcondition
const auto kw_until     = mkkw("until");     // postcondition expression
const auto kw_while     = mkkw("while");     // loop with precondition
const auto kw_for       = mkkw("for");       // range loop
const auto kw_in        = mkkw("in");        // range expression
const auto kw_match     = mkkw("match");     // match expression
const auto kw_of        = mkkw("of");        // range expression
const auto kw_nil       = mkkw("nil");       // none type literal

// special operators

const auto op_call              = mkop("()"); // a(b, c) -emit-> @call(a, tuple&(b, c))
const auto op_subscript         = mkop("[]"); // indexing
const auto op_symbol_resolution = mkop("::"); // symbol resolution
const auto op_member_access     = mkop(".");  // value semantic field access
const auto op_is_truly          = mkop("?");  // ask a thing if it truly or falsy
const auto op_unsafe_unwrap     = mkop("!");  // force get underalying value or borrowed ref
const auto op_safe_navigate     = mkop("?."); // safe navigate
const auto op_navigate          = mkop("!."); // unsafe navigation
const auto op_null_coalescing   = mkop("?:"); // if a - nullable number, a?:0 -> if (a?) a! else 0;
const auto op_pipeline          = mkop("|>"); // chain call operator. a |> b(c) |> d -emit> d(b(a,c))
const auto op_tap               = mkop("=>"); // tap to block

const auto init_expr           = x3::lit("=") >> expr;
const auto type_decl_def       = identifier[type_decl_parsed];
const auto identifier_def      = x3::lexeme[(x3::alpha | x3::char_('_')) >> *(x3::alnum | x3::char_('_'))];
const auto identifier_list_def = (identifier % ',');
const auto var_def_def =
        x3::with<mutable_tag>(var_decl_attr {})[(
                (kw_let | kw_var[make_mutable])
                >> identifier_list
                >> ":"
                >> type_decl
                >> -("=" >> expr)
        )[data_object_decl_parsed]];
const auto fun_decl_def  = (kw_function >> identifier >> '(' >> identifier_list >> ')')[fun_decl_parsed];
const auto fun_block_def = (expr[emplace_to_vec]) % ';';
const auto fun_def_def   = (fun_decl >> fun_block >> kw_end)[fun_def_parsed];
const auto fun_call_def  = (identifier >> '(' >> -expr_list >> ')')[fun_call_parsed];
const auto variable_def  = (identifier >> !x3::lit('('))[variable_parsed];
const auto number_zero   = x3::char_('0') >> !(x3::digit | x3::char_('.'));
const auto numeric_float = x3::lexeme[x3::char_('.') >> *x3::digit];
const auto numeric_dec   = x3::lexeme[((x3::digit - '0') >> *x3::digit) >> -numeric_float];
const auto number_def    = (number_zero | numeric_dec)[number_lit_parsed];
const auto string_def    = x3::lexeme[x3::lit('\"') >> *(x3::char_ - '\"') >> "\""][string_parsed];

const auto op = x3::lexeme[!identifier >> +(x3::char_ - x3::space - x3::digit - x3::alpha - '(' - ')' - ',' - '"' - '\'' - '\\' - ';' - ':' - '.')];

const auto atom_def =
        (number
         | string)[variant_node_upcast];

const auto expr_grouped_def =
        (x3::lit('(') >> expr >> ')')[node_upcast];

const auto simple_def =
        (fun_call
         | atom
         | variable
         | expr_grouped)[variant_node_upcast];

const auto postfix_expr_def =
        (x3::lexeme[simple >> -op])[postfix_expr_parsed];

const auto prefix_expr_def =
        (x3::lexeme[-op >> postfix_expr])[prefix_expr_parsed];

const auto infix_op = x3::no_skip[x3::omit[x3::space] >> op >> x3::omit[x3::space]];

// Бинарные выражения
const auto term_def        = prefix_expr[node_upcast];
const auto expr_next_def   = (infix_op >> term)[expr_next_parsed];
const auto expr_def        = (term >> *(expr_next))[expr_parsed];
const auto expr_list_delim = x3::no_skip[x3::omit[*x3::space] >> x3::lit(',') >> x3::omit[*x3::space]];
const auto expr_list_def   = expr[emplace_to_vec] % expr_list_delim;

const auto stmt_def      = (fun_def | fun_decl | var_def | expr)[variant_node_upcast];
const auto stmt_list_def = stmt[emplace_to_vec] % ';';
const auto chunk_def     = stmt_list;

BOOST_SPIRIT_DEFINE(
        chunk,
        stmt_list,
        stmt,
        type_decl,
        var_def,
        fun_decl,
        fun_block,
        fun_def,
        expr,
        expr_grouped,
        term,
        postfix_expr,
        prefix_expr,
        expr_next,
        expr_list,
        atom,
        simple,
        fun_call,
        variable,
        number,
        string,
        identifier_list,
        identifier
);

const auto comment = x3::lexeme["#" >> *(x3::char_ - x3::eol)] >> (x3::eol | x3::eoi);
const auto skipper = x3::space | comment;

} // namespace kaleidoscope::parser::grammar
