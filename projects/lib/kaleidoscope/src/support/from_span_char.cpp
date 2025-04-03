#include <kaleidoscope/parser/support/from_span_char.hpp>
#include <kaleidoscope/parser/details/impl_x3.hpp>

namespace kaleidoscope
{

template<>
Parser::Result
Parser::parse<SpanIterator>(SpanIterator begin, SpanIterator end)
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
