#pragma once

#include <kaleidoscope/parser/details/ast_fusion_adapt.hpp>
#include <type_traits>

#include <boost/spirit/home/x3.hpp>

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
    _val(ctx).reset(new ast::Functional_Call(at_c<0>(_attr(ctx)), std::move(at_c<1>(_attr(ctx)))));
};

const auto expr_next_parsed = [](auto &ctx)
{
    _val(ctx) = std::move(std::make_pair(at_c<0>(_attr(ctx)), std::move(at_c<1>(_attr(ctx)))));
};

const auto expr_parsed = [](auto &ctx)
{
    auto &first      = at_c<0>(_attr(ctx));
    auto &operations = at_c<1>(_attr(ctx));

    _val(ctx).reset(new ast::Precedence_Agnostic_Expr(
            std::move(first),
            std::move(operations)
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
const x3::rule<class R_Chunk,     std::vector< std::unique_ptr< ast::Node                     >>> chunk           = "chunk";
const x3::rule<class R_Stmt_List, std::vector< std::unique_ptr< ast::Node                     >>> stmt_list       = "statement-list";
const x3::rule<class R_Stmt,                   std::unique_ptr< ast::Node                      >> stmt            = "statement";
const x3::rule<class R_Fun_Decl,               std::unique_ptr< ast::Function_Declaration      >> fun_decl        = "function-declaration";
const x3::rule<class R_Fun_Def,                std::unique_ptr< ast::Function_Defenition       >> fun_def         = "function-defenition";
const x3::rule<class R_Fun_Def,   std::vector< std::unique_ptr< ast::Node                     >>> expr_list       = "expression-list";
const x3::rule<class R_Expr,                   std::unique_ptr< ast::Precedence_Agnostic_Expr  >> expr            = "expression-raw";
const x3::rule<class R_Expr_Next,   std::pair<
                                               ast::Precedence_Agnostic_Expr::Op,
                                               std::unique_ptr< ast::Node >
                                             >                                                  > expr_next       = "expression-continue";
const x3::rule<class R_Simple,                 std::unique_ptr< ast::Node                      >> simple          = "simple";
const x3::rule<class R_Fun_Call,               std::unique_ptr< ast::Functional_Call           >> fun_call        = "functional-call";
const x3::rule<class R_Var,                    std::unique_ptr< ast::Variable                  >> variable        = "variable";
const x3::rule<class R_Number,                 std::unique_ptr< ast::Lexeme_Numeric            >> number          = "number";
const x3::rule<class R_Identifier_List,        std::vector<     std::string                    >> identifier_list = "identifier-list";
const x3::rule<class R_Identifier,                              std::string                     > identifier      = "identifier";
// clang-format on

auto mkkw = [](std::string kw)
{
    return x3::lexeme[x3::lit(kw) >> !x3::alnum];
};

const auto kw_def   = mkkw("def");
const auto reserved = kw_def;

const auto identifier_def      = x3::lexeme[(x3::alpha | x3::char_('_')) >> *(x3::alnum | x3::char_('_'))];
const auto identifier_list_def = (identifier % ',');
const auto fun_decl_def        = (x3::lit("def") >> identifier >> '(' >> identifier_list >> ')')[fun_decl_parsed];
const auto fun_def_def         = (fun_decl >> '=' >> expr)[fun_def_parsed];
const auto fun_call_def        = (identifier >> '(' >> expr_list >> ')')[fun_call_parsed];
const auto variable_def        = identifier[variable_parsed];
const auto number_def          = x3::double_[number_parsed];

const auto op = !identifier >> +(x3::char_ - x3::digit - x3::alpha - '(' - ')' - ',' - '"' - '\'' - '\\' - ';');
const auto simple_def =
        (number
         | fun_call
         | variable
         | (x3::lit('(') >> expr >> ')')
        )[variant_node_upcast];

const auto expr_next_def = (op >> simple)[expr_next_parsed];
const auto expr_def      = (simple >> *(expr_next))[expr_parsed];
const auto expr_list_def = (expr[emplace_to_vec] % ',');

// const auto stmt_def      = (fun_def | fun_decl | expr)[variant_node_upcast];
const auto stmt_def      = (fun_decl | expr)[variant_node_upcast];
const auto stmt_list_def = stmt[emplace_to_vec] % ';';
const auto chunk_def     = stmt_list;

BOOST_SPIRIT_DEFINE(
        chunk,
        stmt_list,
        stmt,
        fun_decl,
        expr,
        expr_next,
        expr_list,
        simple,
        fun_call,
        variable,
        number,
        identifier_list,
        identifier
);
// fun_def,

const auto comment = x3::lit('#') >> *(!x3::eol) >> x3::eol;
const auto skipper = x3::ascii::space | comment;

} // namespace kaleidoscope::parser::grammar
