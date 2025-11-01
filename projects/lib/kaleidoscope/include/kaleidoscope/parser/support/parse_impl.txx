#pragma once

#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/details/impl_x3.hpp>

namespace kaleidoscope
{

template<concepts::TriviallyCopyableIterator T>
inline Parser::Result
Parser::parse(T begin, T end)
{
    using namespace boost::spirit::x3;
    Parser::Result result;

    std::vector<std::unique_ptr<ast::Node>> out;
    bool                                    is_full_parsed =
            phrase_parse(
                    begin,
                    end,
                    parser::grammar::chunk,
                    parser::grammar::skipper,
                    out
            )
            && (begin == end);

    std::string_view least {begin, end};

    return Parser::Result(std::move(out));
}

} // namespace kaleidoscope
