#pragma once

#include <kaleidoscope/ast.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Literal_Numeric, value)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Variable, name)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Function_Declaration, name, args)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Function_Defenition, prototype, body)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Functional_Call, callee, args)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Operation_Prefix, operand)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Operation_Infix, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Precedence_Agnostic_Expr, first, operations)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Type_Declaration, name)
BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::ast::Data_Object_Definition_List, names)

BOOST_FUSION_ADAPT_STRUCT(kaleidoscope::Ast, statements)
