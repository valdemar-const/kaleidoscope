#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_char.hpp>
#include <kaleidoscope/ast/stringify.hpp>
#include <kaleidoscope/ast/precedence.hpp>
#include <kaleidoscope/ast/iterator.hpp>
#include <kaleidoscope/state.hpp>

#include <boost/nowide/iostream.hpp>

#define BOOST_TEST_MODULE kaleidoscope_parser
#include <boost/test/included/unit_test.hpp>

using namespace kaleidoscope::ast::utils;
using namespace std::string_literals;
using Kind          = kaleidoscope::state::operator_properties::Kind;
using Associativity = kaleidoscope::state::operator_properties::Associativity;

struct F
{
  public:

    F(void)
        : preprocess(operators)
    {
        BOOST_TEST_MESSAGE("setup fixture");
    }

    ~F(void)
    {
        BOOST_TEST_MESSAGE("teardown fixture");
    }

    std::string
    make_listing(const kaleidoscope::Ast &ast) const
    {
        std::stringstream is;
        for (auto &&node : ast.statements)
        {
            is << to_string(*node) << std::endl;
        }

        return is.str();
    }

  public:

    const precedence::Bin_Op_Precedence operators = {
            std::make_pair(
                    "**",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Right,
                            .precedence    = 0
                    }
            ),
            std::make_pair(
                    "*",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 10
                    }
            ),
            std::make_pair(
                    "/",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 10
                    }
            ),
            std::make_pair(
                    "+",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 20
                    }
            ),
            std::make_pair(
                    "-",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Binary,
                            .associativity = Associativity::Left,
                            .precedence    = 20
                    }
            )
    };

    precedence preprocess;
};

BOOST_FIXTURE_TEST_SUITE(s, F)

BOOST_AUTO_TEST_CASE(parse_numeric_lexeme)
{
    std::string input = R"KALEIDOSCOPE(
                    def foo(a, b, c);
                    1;
                    1 - 2 - 3 ** 5 ** 6 - 4;
                    5 + 5 * 2 - 1;
                    g + -(7 + b) * -1;
                    a + (b - c * foo(1 + foo(3, 2, a - c), 2, 3) - g) / f;
                    1;
                    def foo(a, b, c)
                        (a - b) * c;
                        a - b * c
                    end
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((typeid(*result.statements.at(1)) == typeid(kaleidoscope::ast::Lexeme_Numeric)));
}

BOOST_AUTO_TEST_SUITE_END()
