#pragma once

#include <kaleidoscope/parser/details/ast_fusion_adapt.hpp>

#include <boost/spirit/home/x3.hpp>

namespace kaleidoscope::parser::actions
{
using boost::fusion::at_c;

const auto node_upcast = [](auto &ctx)
{
    _val(ctx).reset(static_cast<ast::Node *>(_attr(ctx).release()));
};

const auto emplace_to_vec = [](auto &ctx)
{
    _val(ctx).emplace_back(_attr(ctx).release());
};

const auto variant_node_upcast = [](auto &ctx)
{
    boost::apply_visitor(
            [&](auto &&v)
            {
                _val(ctx).reset(static_cast<ast::Node *>(v.release()));
            },
            _attr(ctx)
    );
};

const auto number_parsed = [](auto &ctx)
{
    _val(ctx).reset(new ast::Lexeme_Numeric(_attr(ctx)));
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
    _val(ctx).reset(new ast::Function_Defenition(at_c<0>(_attr(ctx)), at_c<1>(_attr(ctx))));
};

const auto fun_call_parsed = [](auto &ctx)
{
    _val(ctx).reset(new ast::Functional_Call(at_c<0>(_attr(ctx)), at_c<1>(_attr(ctx))));
};

const auto chunk_parsed = [](auto &ctx)
{
    _val(ctx).reset(new Ast(_attr(ctx)));
};

}; // namespace kaleidoscope::parser::actions

namespace kaleidoscope::parser::grammar
{
namespace x3 = boost::spirit::x3;
using namespace actions;

// clang-format off
const x3::rule<class R_Chunk,    std::vector< std::unique_ptr< ast::Node                     >>> chunk     = "chunk";
const x3::rule<class R_Stmt,     std::vector< std::unique_ptr< ast::Node                     >>> stmt_list = "statement-list";
const x3::rule<class R_Stmt,                  std::unique_ptr< ast::Node                      >> stmt      = "statement";
const x3::rule<class R_Fun_Decl,              std::unique_ptr< ast::Function_Declaration      >> fun_decl  = "function-declaration";
const x3::rule<class R_Fun_Def,               std::unique_ptr< ast::Function_Defenition       >> fun_def   = "function-defenition";
const x3::rule<class R_Expr,                  std::unique_ptr< ast::Precedence_Agnostic_Expr  >> expr      = "expression-raw";
const x3::rule<class R_Simple,                std::unique_ptr< ast::Node                      >> simple    = "simple";
const x3::rule<class R_Fun_Call,              std::unique_ptr< ast::Functional_Call           >> fun_call  = "functional-call";
const x3::rule<class R_Var,                   std::unique_ptr< ast::Variable                  >> variable  = "variable";
const x3::rule<class R_Number,                std::unique_ptr< ast::Lexeme_Numeric            >> number    = "number";
// clang-format on

const auto identifier = +(x3::ascii::alpha);

const auto fun_decl_def = (x3::lit("def") >> identifier >> '(' >> (variable % ',') >> ')')[fun_decl_parsed];
const auto fun_def_def  = (fun_decl >> '=' >> expr)[fun_def_parsed];                 // TODO: body???
const auto fun_call_def = (variable >> '(' >> (expr % ',') >> ')')[fun_call_parsed]; // TODO: arg list???
const auto variable_def = identifier[variable_parsed];
const auto number_def   = x3::double_[number_parsed];

const auto op = *x3::char_("+-*/");
const auto simple_def =
        (number
         | fun_call
         | variable
         | (x3::lit('(') >> expr >> ')'))[variant_node_upcast];

const auto expr_def = simple >> *(op >> simple);

// const auto stmt_def      = (fun_def | fun_decl | expr)[variant_node_upcast];
const auto stmt_def      = number[node_upcast];
const auto stmt_list_def = stmt[emplace_to_vec] % ';';
const auto chunk_def     = stmt_list;

BOOST_SPIRIT_DEFINE(
        chunk,
        stmt_list,
        stmt,
        // fun_decl,
        // fun_def,
        // expr,
        // simple,
        // fun_call,
        // variable,
        number
);

const auto comment = x3::lit('#') >> *(!x3::eol) >> x3::eol;
const auto skipper = x3::ascii::space | comment;

} // namespace kaleidoscope::parser::grammar
