#include <kaleidoscope/parser/support/from_span_uint8_t.hpp>
#include <kaleidoscope/parser/details/impl_x3.hpp>

namespace kaleidoscope
{

template<>
Ast
Parser::parse<SpanIterator>(SpanIterator begin, SpanIterator end)
{
    using namespace boost::spirit::x3;

    Ast out;
    bool is_full_parsed = phrase_parse(begin, end, parser::grammar::chunk, parser::grammar::skipper, out) && (begin == end);
    return out;
}

} // namespace kaleidoscope
