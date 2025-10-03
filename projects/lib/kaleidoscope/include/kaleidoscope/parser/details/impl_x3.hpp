#pragma once

#include <kaleidoscope/parser/details/ast_fusion_adapt.hpp>
#include <type_traits>

#include <numeric>
#include <iostream>

#include <boost/spirit/home/x3.hpp>

namespace kaleidoscope::parser::actions
{
using boost::fusion::at_c;

const auto emplace_to_vec = [](auto &ctx)
{
    _val(ctx).emplace_back(std::move(_attr(ctx)));
};

const auto variant_node_upcast = [](auto &ctx)
{
    boost::apply_visitor(
            [&](auto &&v)
            {
                _val(ctx) = std::move(v);
            },
            _attr(ctx)
    );
};

const auto number_parsed = [](auto &ctx)
{
    _val(ctx) = ast::Lexeme_Numeric(_attr(ctx));
};

const auto variable_parsed = [](auto &ctx)
{
    _val(ctx) = ast::Variable(_attr(ctx));
};

const auto fun_decl_parsed = [](auto &ctx)
{
    _val(ctx) = ast::Function_Declaration(at_c<0>(_attr(ctx)), at_c<1>(_attr(ctx)));
};

const auto fun_def_parsed = [](auto &ctx)
{
    auto &func_stmts = at_c<1>(_attr(ctx));

    ast::Function_Defenition::Body statements;
    statements.reserve(func_stmts.size());

    std::transform(
            func_stmts.begin(),
            func_stmts.end(),
            std::back_inserter(statements),
            [](auto &elem)
            {
                return ast::Function_Defenition::Statement {std::move(elem)};
            }
    );
    _val(ctx) = ast::Function_Defenition {
            std::make_unique<ast::Function_Declaration>(at_c<0>(_attr(ctx))),
            std::move(statements)
    };
};

const auto fun_call_parsed = [](auto &ctx)
{
    _val(ctx) = ast::Functional_Call(at_c<0>(_attr(ctx)), std::move(at_c<1>(_attr(ctx))));
};

const auto expr_next_parsed = [](auto &ctx)
{
#if 1
    auto &ops     = at_c<0>(_attr(ctx));
    auto &operand = at_c<1>(_attr(ctx));
    if (ops.size() == 1)
    {
        _val(ctx) = std::make_pair(ops.front(), std::move(operand));
    }
    else if (ops.size() == 2)
    {
        _val(ctx) = std::make_pair(ops.front(), ast::INode {ast::Operation_Unary(ops.back(), std::move(operand))});
    }
    else
    {
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
    auto &first      = at_c<0>(_attr(ctx));
    auto &operations = at_c<1>(_attr(ctx));

    _val(ctx) = ast::Precedence_Agnostic_Expr(
            std::move(first),
            std::move(operations)
    );
};

const auto chunk_parsed = [](auto &ctx)
{
    _val(ctx) = Ast(std::move(_attr(ctx)));
};

}; // namespace kaleidoscope::parser::actions

namespace kaleidoscope::parser::grammar
{
namespace x3 = boost::spirit::x3;
using namespace actions;

// clang-format off
const x3::rule<class R_Chunk,     std::vector< ast::INode                                      >> chunk           = "chunk";
const x3::rule<class R_Stmt_List, std::vector< ast::INode                                      >> stmt_list       = "statement-list";
const x3::rule<class R_Stmt,                   ast::INode                                       > stmt            = "statement";
const x3::rule<class R_Fun_Decl,               ast::Function_Declaration                        > fun_decl        = "function-declaration";
const x3::rule<class R_Fun_Block, std::vector< ast::INode                                      >> fun_block       = "function-block";
const x3::rule<class R_Fun_Def,                ast::Function_Defenition                         > fun_def         = "function-defenition";
const x3::rule<class R_Fun_Def,   std::vector< ast::INode                                      >> expr_list       = "expression-list";
const x3::rule<class R_Expr,                   ast::Precedence_Agnostic_Expr                    > expr            = "expression-raw";
const x3::rule<class R_Expr_Next,   std::pair<
                                               ast::Precedence_Agnostic_Expr::Op,
                                               ast::INode
                                             >                                                  > expr_next       = "expression-continue";
const x3::rule<class R_Simple,                 ast::INode                                       > simple          = "simple";
const x3::rule<class R_Fun_Call,               ast::Functional_Call                             > fun_call        = "functional-call";
const x3::rule<class R_Var,                    ast::Variable                                    > variable        = "variable";
const x3::rule<class R_Number,                 ast::Lexeme_Numeric                              > number          = "number";
const x3::rule<class R_Identifier_List,        std::vector<     std::string                    >> identifier_list = "identifier-list";
const x3::rule<class R_Identifier,                              std::string                     > identifier      = "identifier";

// clang-format on

template<typename S>
auto
mkkw(S &&kw)
{
    return x3::lexeme[x3::lit(std::string(kw)) >> !x3::alnum];
}

const auto kw_def   = mkkw("def");
const auto kw_end   = mkkw("end");
const auto reserved = kw_def | kw_end;

const auto identifier_def      = x3::lexeme[(x3::alpha | x3::char_('_')) >> *(x3::alnum | x3::char_('_'))];
const auto identifier_list_def = (identifier % ',');
const auto fun_decl_def        = (kw_def >> identifier >> '(' >> identifier_list >> ')')[fun_decl_parsed];
const auto fun_block_def       = (expr[emplace_to_vec]) % ';';
const auto fun_def_def         = (fun_decl >> fun_block >> kw_end)[fun_def_parsed];
const auto fun_call_def        = (identifier >> '(' >> expr_list >> ')')[fun_call_parsed];
const auto variable_def        = identifier[variable_parsed];
const auto number_def          = x3::double_[number_parsed];

const auto op = +x3::lexeme[!identifier >> +(x3::char_ - x3::space - x3::digit - x3::alpha - '(' - ')' - ',' - '"' - '\'' - '\\' - ';')];
const auto simple_def =
        (number
         | fun_call
         | variable
         | (x3::lit('(') >> expr >> ')'))[variant_node_upcast];

const auto expr_next_def = (op >> simple)[expr_next_parsed];
const auto expr_def      = (simple >> *(expr_next))[expr_parsed];
const auto expr_list_def = (expr[emplace_to_vec] % ',');

const auto stmt_def      = (fun_def | fun_decl | expr)[variant_node_upcast];
const auto stmt_list_def = stmt[emplace_to_vec] % ';';
const auto chunk_def     = stmt_list;

BOOST_SPIRIT_DEFINE(
        chunk,
        stmt_list,
        stmt,
        fun_decl,
        fun_block,
        fun_def,
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

const auto comment = x3::lit('#') >> *(!x3::eol) >> x3::eol;
const auto skipper = x3::ascii::space | comment;

} // namespace kaleidoscope::parser::grammar
