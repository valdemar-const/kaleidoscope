#include <kaleidoscope/parser/support/parse_impl.txx>
#include <kaleidoscope/parser/support/from_string_view.hpp>
#include <kaleidoscope/parser/details/impl_x3.hpp>

#include <span>

namespace kaleidoscope
{
template<>
Parser::Result
Parser::parse<ParserStringViewIterator>(ParserStringViewIterator begin, ParserStringViewIterator end)
{
    using SpanType = std::span<const char>;
    SpanType data(begin, static_cast<SpanType::size_type>(std::distance(begin, end)));
    return parse(data.begin(), data.end());
}

template Parser::Result Parser::parse<ParserStringViewIterator>(ParserStringViewIterator begin, ParserStringViewIterator end);

} // namespace kaleidoscope
