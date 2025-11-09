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

#define BOOST_TEST_MODULE kaleidoscope_parser
#include <boost/test/included/unit_test.hpp>

using namespace kaleidoscope::ast::utils;
using namespace std::string_literals;
using Kind          = kaleidoscope::state::operator_properties::Kind;
using Associativity = kaleidoscope::state::operator_properties::Associativity;

namespace
{

struct ast_to_string
{
    using Result = std::string;

    Result
    operator()(const kaleidoscope::ast::Literal_Numeric &node) const
    {
        return std::visit([](const auto &value) -> std::string
                          {
                              return std::to_string(value);
                          },
                          node.value);
    }
};

int example(int, double);

} // namespace

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
                            .kind          = Kind::Infix,
                            .associativity = Associativity::Right,
                            .precedence    = 0
                    }
            ),
            std::make_pair(
                    "*",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Infix,
                            .associativity = Associativity::Left,
                            .precedence    = 10
                    }
            ),
            std::make_pair(
                    "/",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Infix,
                            .associativity = Associativity::Left,
                            .precedence    = 10
                    }
            ),
            std::make_pair(
                    "+",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Infix,
                            .associativity = Associativity::Left,
                            .precedence    = 20
                    }
            ),
            std::make_pair(
                    "-",
                    kaleidoscope::state::operator_properties {
                            .kind          = Kind::Infix,
                            .associativity = Associativity::Left,
                            .precedence    = 20
                    }
            )
    };

    precedence preprocess;
};

BOOST_FIXTURE_TEST_SUITE(s, F)

BOOST_AUTO_TEST_CASE(parse_atom)
{
    std::string input = R"KALEIDOSCOPE(
                    1
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST((result.statements.size() == 1));
    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Literal_Numeric)));
}

BOOST_AUTO_TEST_CASE(parse_prefix_op)
{
    std::string input = R"KALEIDOSCOPE(
                    !1
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST((result.statements.size() == 1));
    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Operation_Prefix)));
}

BOOST_AUTO_TEST_CASE(parse_postfix_op)
{
    std::string input = R"KALEIDOSCOPE(
                    1!
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST((result.statements.size() == 1));
    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Operation_Postfix)));
}

BOOST_AUTO_TEST_CASE(parse_postfix_prefix)
{
    std::string input = R"KALEIDOSCOPE(
                    !1!
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST((result.statements.size() == 1));
    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Operation_Prefix)));
}

BOOST_AUTO_TEST_CASE(parse_infix_prefix)
{
    std::string input = R"KALEIDOSCOPE(
                    !(1 + 2);
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    BOOST_TEST_MESSAGE(make_listing(result));
    preprocess(result);
    BOOST_TEST_MESSAGE(make_listing(result));
    BOOST_TEST((result.statements.size() == 1));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Operation_Prefix)));
}

BOOST_AUTO_TEST_CASE(parse_foo_args)
{
    std::string input = R"KALEIDOSCOPE(
                    foo;
                    foo();
                    foo(1);
                    foo(1,2);
                    foo(1,2,3);
                    foo(1, 2);
                    foo(1, 2, 3);
                    foo(1 , 2);
                    foo(1 , 2 , 3);
                    foo(1 + 2);
                    foo(foo(-1+) + 2);
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    BOOST_TEST_MESSAGE(make_listing(result));
    preprocess(result);
    BOOST_TEST_MESSAGE(make_listing(result));
    BOOST_TEST((result.statements.size() == 11));

    BOOST_TEST((typeid(*result.statements.at(1)) == typeid(kaleidoscope::ast::Functional_Call)));
}

BOOST_AUTO_TEST_CASE(parse_numeric_lexeme)
{
    std::string input = R"KALEIDOSCOPE(
                    # this is a single line commentary
                    function foo(a, b, c);                                 # function declaration
                    1;                                                     # numeric lexeme
                    "3";
                    1 - 2 - 3 ** 5 ** 6 - 4;                               # mathematical expression 1
                    5 + 5 * 2 - 1;                                         # mathematical expression 1
                    g + -(7 + b) * -1;                                     # mathematical expression 1
                    a;                                                     # mathematical expression 1
                    a + b;                                                 # mathematical expression 1
                    foo(1, 2, 3);                                          # mathematical expression 1
                    a + (b - c * foo(1 + foo(3, 2, a - c), 2, 3) - g) / f; # mathematical expression 1
                    # function definition
                    function foo(a, b, c)
                        (a - b) * c;
                        a - b * c
                    end
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST_MESSAGE(make_listing(result));
    BOOST_TEST((result.statements.size() == 11));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Function_Declaration)));
    BOOST_TEST((typeid(*result.statements.at(1)) == typeid(kaleidoscope::ast::Literal_Numeric)));
    BOOST_TEST((typeid(*result.statements.at(2)) == typeid(kaleidoscope::ast::Literal_String)));
    BOOST_TEST((typeid(*result.statements.at(3)) == typeid(kaleidoscope::ast::Operation_Infix)));
    BOOST_TEST((typeid(*result.statements.at(10)) == typeid(kaleidoscope::ast::Function_Defenition)));
}

BOOST_AUTO_TEST_CASE(parse_data_objects_definitions)
{
    std::string defvar = R"KALEIDOSCOPE(
                var num, num2 : int; # mutable
                let num_: int; # immutable
                1;
                1 + -1;
                !1;
                !0;
            )KALEIDOSCOPE"s; // num: 0 - by default

    auto result = kaleidoscope::Parser::parse(defvar.begin(), defvar.end());
    preprocess(result);
    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((6 == result.statements.size()));
    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Data_Object_Definition_List)));
    BOOST_TEST((typeid(*result.statements.at(1)) == typeid(kaleidoscope::ast::Data_Object_Definition_List)));
    BOOST_TEST((typeid(*result.statements.at(2)) == typeid(kaleidoscope::ast::Literal_Numeric)));
    BOOST_TEST_MESSAGE(compiler::demangle(typeid(*result.statements.at(2)).name()));
    BOOST_TEST((typeid(*result.statements.at(3)) == typeid(kaleidoscope::ast::Operation_Infix)));

    BOOST_TEST((typeid(*result.statements.at(4)) == typeid(kaleidoscope::ast::Operation_Prefix)));
    BOOST_TEST((typeid(*result.statements.at(5)) == typeid(kaleidoscope::ast::Operation_Prefix)));
}

BOOST_AUTO_TEST_CASE(functional_traits)
{
    using Args           = boost::callable_traits::args_t<decltype(example)>; // std::tuple<int, double>
    using Result         = boost::callable_traits::return_type_t<decltype(example)>;
    constexpr auto arity = std::tuple_size_v<Args>;

    std::string result_typename = compiler::demangle(typeid(Result).name());

    static constexpr auto get_args_signatures = []<typename Tuple>(void) -> const std::vector<std::type_index> &
    {
        // for type in Tuple do args_type_signatures.emplace_back(typeid(type)); done
        static const auto result = []<size_t... I>(std::index_sequence<I...>) -> std::vector<std::type_index>
        {
            std::vector<std::type_index> args_type_signatures;
            (args_type_signatures.emplace_back(typeid(std::tuple_element_t<I, Tuple>)), ...);
            return args_type_signatures;
        }(std::make_index_sequence<std::tuple_size_v<Tuple>> {});
        return result;
    };

    auto args_type_signatures = get_args_signatures.operator()<Args>();

    auto all_typenames = std::accumulate(args_type_signatures.begin(), args_type_signatures.end(), std::string {}, [](auto acc, auto elem)
                                         {
                                             return (acc.empty()) ? ("\"" + compiler::demangle(elem.name()) + "\"")
                                                                  : (acc + ", \"" + compiler::demangle(elem.name()) + "\"");
                                         });

    BOOST_TEST_MESSAGE(("{\"type\": \"functional\", \"return\": \"" + result_typename + "\", \"args\": [" + all_typenames + "]}"));
}

BOOST_AUTO_TEST_SUITE_END()
