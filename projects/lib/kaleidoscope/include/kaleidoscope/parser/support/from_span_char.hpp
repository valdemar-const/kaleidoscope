#include <kaleidoscope/parser.hpp>
#include <span>
#include <iterator>
#include <cinttypes>

namespace kaleidoscope
{
using ParserSpanCharIterator      = std::span<char>::iterator;
using ParserSpanCharIteratorConst = std::span<const char>::iterator;
} // namespace kaleidoscope
