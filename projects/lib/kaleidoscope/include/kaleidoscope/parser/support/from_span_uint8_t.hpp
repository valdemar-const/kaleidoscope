#include <kaleidoscope/parser.hpp>
#include <span>
#include <iterator>
#include <cinttypes>

namespace kaleidoscope
{
using ParserSpanByteIterator      = std::span<uint8_t>::iterator;
using ParserSpanByteIteratorConst = std::span<const uint8_t>::iterator;

template<>
Parser::Result
Parser::parse<ParserSpanByteIterator>(ParserSpanByteIterator begin, ParserSpanByteIterator end);

template<>
Parser::Result
Parser::parse<ParserSpanByteIteratorConst>(ParserSpanByteIteratorConst begin, ParserSpanByteIteratorConst end);
} // namespace kaleidoscope
