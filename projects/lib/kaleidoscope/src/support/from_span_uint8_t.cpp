#include <kaleidoscope/parser/support/parse_impl.txx>
#include <kaleidoscope/parser/support/from_span_uint8_t.hpp>
#include <kaleidoscope/parser/details/impl_x3.hpp>

namespace kaleidoscope
{
template<>
Parser::Result
Parser::parse<ParserSpanByteIterator>(ParserSpanByteIterator begin, ParserSpanByteIterator end)
{
    using ElementType = std::remove_cv_t<ParserSpanByteIterator>::value_type;
    using SpanType    = std::span<ElementType>;
    using Size_Type   = SpanType::size_type;

    auto     size = static_cast<Size_Type>(std::distance(begin, end));
    SpanType data {&(*begin), size};

    return parse(data.begin(), data.end());
}

template<>
Parser::Result
Parser::parse<ParserSpanByteIteratorConst>(ParserSpanByteIteratorConst begin, ParserSpanByteIteratorConst end)
{
    using ElementType = std::remove_cv_t<ParserSpanByteIterator>::value_type;
    using SpanType    = std::span<const ElementType>;
    using Size_Type   = SpanType::size_type;

    auto     size = static_cast<Size_Type>(std::distance(begin, end));
    SpanType data {&(*begin), size};

    return parse(data.begin(), data.end());
}

template Parser::Result Parser::parse<ParserSpanByteIterator>(ParserSpanByteIterator begin, ParserSpanByteIterator end);
template Parser::Result Parser::parse<ParserSpanByteIteratorConst>(ParserSpanByteIteratorConst begin, ParserSpanByteIteratorConst end);
} // namespace kaleidoscope
