#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/parse_impl.txx>

#include <string_view>
#include <iterator>
#include <cinttypes>

namespace kaleidoscope
{
using ParserStringViewIterator = std::string_view::iterator;

template<>
Parser::Result
Parser::parse<ParserStringViewIterator>(ParserStringViewIterator begin, ParserStringViewIterator end);

} // namespace kaleidoscope
