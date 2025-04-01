#pragma once

#include <kaleidoscope/ast.hpp>

namespace kaleidoscope
{

struct Parser
{
    template<typename Iterator>
    static Ast parse(Iterator begin, Iterator end);
};

} // namespace kaleidoscope
