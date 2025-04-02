#pragma once

#include <kaleidoscope/ast.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Lexeme_Numeric, value)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Variable, name)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Function_Declaration, name, args)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Function_Defenition, prototype, body)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Functional_Call, callee, args)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Operation_Binary, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Precedence_Agnostic_Expr, first, operations)

BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::Ast, statements)
