#pragma once

#include <kaleidoscope/parser/details/ast_fusion_adapt.hpp>

#include <boost/spirit/home/x3.hpp>

namespace kaleidoscope::parser::grammar
{
namespace x3 = boost::spirit::x3;

const x3::rule<class R_Chunk, Ast> chunk = "chunk";

const auto chunk_def = *x3::char_;

BOOST_SPIRIT_DEFINE(chunk);

const auto skipper = x3::ascii::space;

} // namespace kaleidoscope::parser::grammar
