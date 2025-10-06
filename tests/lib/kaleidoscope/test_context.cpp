#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_span_char.hpp>
#include <kaleidoscope/ast/stringify.hpp>
#include <kaleidoscope/ast/precedence.hpp>
#include <kaleidoscope/ast/iterator.hpp>
#include <kaleidoscope/state.hpp>

#include <anyany/anyany.hpp>
#include <anyany/utility.hpp>
#include <anyany/visit_invoke.hpp>
#include <anyany/type_descriptor.hpp>

#include <functional>
#include <unordered_set>

#include <boost/nowide/iostream.hpp>

#include <boost/type_traits/function_traits.hpp>
#include <boost/callable_traits.hpp>

#define BOOST_TEST_MODULE kaleidoscope_context
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

    kaleidoscope::state ctx;
};

BOOST_FIXTURE_TEST_SUITE(s, F)

BOOST_AUTO_TEST_CASE(eval_lexemes)
{
    std::string input = R"KALEIDOSCOPE(
                    # this is a single line commentary
                    def foo(a, b, c);                                      # function declaration
                    1;                                                     # numeric lexeme
                    1 - 2 - 3 ** 5 ** 6 - 4;                               # mathematical expression 1
                    5 + 5 * 2 - 1;                                         # mathematical expression 1
                    g + -(7 + b) * -1;                                     # mathematical expression 1
                    a + (b - c * foo(1 + foo(3, 2, a - c), 2, 3) - g) / f; # mathematical expression 1
                    # function definition
                    def foo(a, b, c)
                        (a - b) * c;
                        a - b * c
                    end
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST(result.statements.size() == 7);
    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Function_Declaration)));
    BOOST_TEST((typeid(*result.statements.at(1)) == typeid(kaleidoscope::ast::Lexeme_Numeric)));
    BOOST_TEST((typeid(*result.statements.at(2)) == typeid(kaleidoscope::ast::Operation_Binary)));
    BOOST_TEST((typeid(*result.statements.at(6)) == typeid(kaleidoscope::ast::Function_Defenition)));
}

BOOST_AUTO_TEST_SUITE_END()
