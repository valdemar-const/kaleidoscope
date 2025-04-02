#include <kaleidoscope/parser.hpp>
#include <span>
#include <iterator>
#include <cinttypes>

namespace kaleidoscope
{
using SpanIterator = std::span<const uint8_t>::iterator;

template<>
Parser::Result
Parser::parse<SpanIterator>(SpanIterator begin, SpanIterator end);
} // namespace kaleidoscope
