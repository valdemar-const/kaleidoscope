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

template<typename Derived, typename Value>
struct Type_Switch_CRTP
{
    using value_t  = Value;
    using result_t = std::optional<value_t>;
    using Any      = aa::poly_ref_t<>;
    using Visit    = std::function<result_t(Any)>;

  protected:

    Type_Switch_CRTP(void)
        : visit_chain_(return_default_)
    {
    }

  public:

    template<typename T, typename F>
    Type_Switch_CRTP &
    register_handler(F &&handler)
    {
        if (registered_handlers_.contains(aa::descriptor_v<T>))
        {
            return *this;
        }
        else
        {
            registered_handlers_.insert(aa::descriptor_v<T>);
        }

        auto binded_forward = [this, handler = std::forward<F>(handler)](Any any)
        {
            return aa::type_switch<result_t>(any)
                    .template case_<std::decay_t<T>>(handler)
                    .default_(visit_chain_(any));
        };

        visit_chain_ = binded_forward;
        return *this;
    }

  protected:

    result_t
    operator()(Any concrete)
    {
        return visit_chain_(static_cast<Any>(concrete));
    }

  protected:

    Visit                                visit_chain_;
    std::unordered_set<aa::descriptor_t> registered_handlers_;

  private:

    static result_t
    return_default_(Any)
    {
        return std::nullopt;
    }
};

struct Ast_To_String : public Type_Switch_CRTP<Ast_To_String, std::string>
{
    using Super = Type_Switch_CRTP<Ast_To_String, std::string>;

    Ast_To_String(void)
        : Super()
    {
    }

    template<typename T>
    result_t
    operator()(T &&v)
    {
        return Super::operator()(static_cast<Any>(v));
    }

    result_t
    operator()(const kaleidoscope::ast::Literal_Numeric &node)
    {
        return std::visit([](const auto &value) -> std::string
                          {
                              return std::to_string(value) + "(static dispatch)";
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

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Operation_Infix)));
}

BOOST_AUTO_TEST_CASE(parse_numeric_lexeme)
{
    std::string input = R"KALEIDOSCOPE(
                    # this is a single line commentary
                    function foo(a, b, c);                                      # function declaration
                    1;                                                     # numeric lexeme
                    "3";
                    1 - 2 - 3 ** 5 ** 6 - 4;                               # mathematical expression 1
                    5 + 5 * 2 - 1;                                         # mathematical expression 1
                    g + -(7 + b) * -1;                                     # mathematical expression 1
                    a + (b - c * foo(1 + foo(3, 2, a - c), 2, 3) - g) / f; # mathematical expression 1
                    # function definition
                    function foo(a, b, c)
                        (a - b) * c;
                        a - b * c
                    end
                )KALEIDOSCOPE"s;

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST((result.statements.size() == 8));
    BOOST_TEST_MESSAGE(make_listing(result));

    BOOST_TEST((typeid(*result.statements.at(0)) == typeid(kaleidoscope::ast::Function_Declaration)));
    BOOST_TEST((typeid(*result.statements.at(1)) == typeid(kaleidoscope::ast::Literal_Numeric)));
    BOOST_TEST((typeid(*result.statements.at(2)) == typeid(kaleidoscope::ast::Literal_String)));
    BOOST_TEST((typeid(*result.statements.at(3)) == typeid(kaleidoscope::ast::Operation_Infix)));
    BOOST_TEST((typeid(*result.statements.at(7)) == typeid(kaleidoscope::ast::Function_Defenition)));
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

BOOST_AUTO_TEST_CASE(anyany_check)
{
    std::string input = "5";

    auto result = kaleidoscope::Parser::parse(input.begin(), input.end());
    preprocess(result);

    BOOST_TEST((result.statements.size() == 1));

    kaleidoscope::ast::INodeRef any_ast = *result.statements.front();

    BOOST_TEST((any_ast.type_index() == std::type_index {typeid(*result.statements.front())}));
    BOOST_TEST((any_ast.type_descriptor() == aa::descriptor_v<kaleidoscope::ast::Literal_Numeric>));

    ast_to_string visitor {};

    auto listing =
            aa::type_switch<std::optional<std::string>>(any_ast)
                    .case_<const kaleidoscope::ast::Literal_Numeric &>(visitor)
                    .default_(std::nullopt); // good

    auto stringify_visitor = aa::make_visit_invoke<std::string>(
            [](ast_to_string &visitor, const kaleidoscope::ast::Literal_Numeric &node)
            {
                return visitor(node);
            }
    );

    auto stringify = [&stringify_visitor, &visitor](auto &&node) -> std::optional<std::string>
    {
        return stringify_visitor.resolve(visitor, std::forward<decltype(node)>(node));
    };

    auto listing2 = stringify(any_ast);

    BOOST_TEST((listing.value() == "5.000000"));
    BOOST_TEST((listing2.value() == listing.value()));
}

BOOST_AUTO_TEST_CASE(functional_traits)
{
    using Example_Func   = boost::function_traits<decltype(example)>;
    constexpr auto arity = Example_Func::arity;

    using Args = boost::callable_traits::args_t<decltype(example)>; // std::tuple<int, double>

    std::string result_typename = compiler::demangle(typeid(Example_Func::result_type).name());

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
