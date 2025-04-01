#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_uint8_t.hpp>

#include <string>

int
main(int argc, const char *argv[])
{
    std::string        input {"some input source"};
    std::span<const uint8_t> input_span {reinterpret_cast<uint8_t *>(input.data()), input.size()};

    auto result = kaleidoscope::Parser::parse(input_span.begin(), input_span.end());
    return 0;
}
