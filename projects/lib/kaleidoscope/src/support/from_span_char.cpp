#include <kaleidoscope/parser/support/parse_impl.txx>
#include <kaleidoscope/parser/support/from_span_char.hpp>
#include <kaleidoscope/parser/details/impl_x3.hpp>

namespace kaleidoscope
{
template Parser::Result Parser::parse<ParserSpanCharIterator>(ParserSpanCharIterator begin, ParserSpanCharIterator end);
template Parser::Result Parser::parse<ParserSpanCharIteratorConst>(ParserSpanCharIteratorConst begin, ParserSpanCharIteratorConst end);
} // namespace kaleidoscope
