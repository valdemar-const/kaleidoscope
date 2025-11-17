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
using Kind          = kaleidoscope::state::Operator_Properties::Kind;
using Associativity = kaleidoscope::state::Operator_Properties::Associativity;

struct F
{
  public:

    F(void)
    {
        BOOST_TEST_MESSAGE("setup fixture");
        ctx.import(build_context());
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

    kaleidoscope::state ctx;

  protected:

    kaleidoscope::Module
    build_context(void)
    {
        using namespace std::string_literals;
        using namespace kaleidoscope;
        using Kind          = kaleidoscope::Module::Operator_Properties::Kind;
        using Associativity = kaleidoscope::Module::Operator_Properties::Associativity;

        ctx.get_rt()
                .register_ast_promotion<kaleidoscope::ast::Literal_Numeric>(
                        [](const kaleidoscope::ast::Literal_Numeric &numeric) -> std::any
                        {
                            return std::visit([](const auto &v) -> std::any
                                              {
                                                  using Number = std::decay_t<decltype(v)>;
                                                  if constexpr (std::is_floating_point_v<Number>)
                                                  {
                                                      return static_cast<double>(v);
                                                  }
                                                  else if constexpr (std::is_signed_v<Number>)
                                                  {
                                                      return static_cast<int64_t>(v);
                                                  }
                                                  else if constexpr (std::is_unsigned_v<Number>)
                                                  {
                                                      return static_cast<uint64_t>(v);
                                                  }

                                                  throw std::runtime_error("unsupported runtime value type: "s + compiler::demangle(typeid(v).name()));
                                              },
                                              numeric.value);
                        }
                )
                .register_ast_promotion<kaleidoscope::ast::Literal_String>(
                        [](const kaleidoscope::ast::Literal_String &str) -> std::string
                        {
                            return str.value;
                        }
                );

        Module context;

        context.bind_type<bool>("bool");
        context.bind_type<int64_t>("i64");
        context.bind_type<uint64_t>("u64");
        context.bind_type<double>("f64");
        context.bind_type<std::string>("string");

        // script::core

        // - unary ops:

        /* prefix + */ {
            context.bind_op(
                    "+",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](int64_t op) -> int64_t
                    {
                        return +op;
                    }
            );

            context.bind_op(
                    "+",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](uint64_t op) -> uint64_t
                    {
                        return +op;
                    }
            );

            context.bind_op(
                    "+",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](double op) -> double
                    {
                        return +op;
                    }
            );
        }

        /* prefix - */ {
            context.bind_op(
                    "-",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](int64_t op) -> int64_t
                    {
                        return -op;
                    }
            );

            context.bind_op(
                    "-",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](uint64_t op) -> uint64_t
                    {
                        return -op;
                    }
            );

            context.bind_op(
                    "-",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](double op) -> double
                    {
                        return -op;
                    }
            );
        }

        /* prefix ! */ {
            context.bind_op(
                    "!",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](int64_t op) -> bool
                    {
                        return !op;
                    }
            );

            context.bind_op(
                    "!",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](uint64_t op) -> bool
                    {
                        return !op;
                    }
            );

            context.bind_op(
                    "!",
                    {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                    [](double op) -> bool
                    {
                        return !op;
                    }
            );
        }

        // - binary ops:

        // '&'
        // '|'
        // '^'
        // '~'
        // '<<'
        // '>>'

        /* infix * */ {
            context.bind_op(
                    "*",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](int64_t lhs, int64_t rhs) -> int64_t
                    {
                        return lhs * rhs;
                    }
            );

            context.bind_op(
                    "*",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](uint64_t lhs, uint64_t rhs) -> uint64_t
                    {
                        return lhs * rhs;
                    }
            );

            context.bind_op(
                    "*",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](double lhs, double rhs) -> double
                    {
                        return lhs * rhs;
                    }
            );
        }

        /* infix / */ {
            context.bind_op(
                    "/",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](int64_t lhs, int64_t rhs) -> int64_t
                    {
                        return lhs / rhs;
                    }
            );

            context.bind_op(
                    "/",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](uint64_t lhs, uint64_t rhs) -> uint64_t
                    {
                        return lhs / rhs;
                    }
            );

            context.bind_op(
                    "/",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](double lhs, double rhs) -> double
                    {
                        return lhs / rhs;
                    }
            );
        }

        /* infix % */ {
            context.bind_op(
                    "%",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](int64_t lhs, int64_t rhs) -> int64_t
                    {
                        return lhs % rhs;
                    }
            );

            context.bind_op(
                    "%",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                    [](uint64_t lhs, uint64_t rhs) -> uint64_t
                    {
                        return lhs % rhs;
                    }
            );
        }

        /* infix + */ {
            context.bind_op(
                    "+",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                    [](int64_t lhs, int64_t rhs) -> int64_t
                    {
                        return lhs + rhs;
                    }
            );

            context.bind_op(
                    "+",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                    [](uint64_t lhs, uint64_t rhs) -> uint64_t
                    {
                        return lhs + rhs;
                    }
            );

            context.bind_op(
                    "+",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                    [](double lhs, double rhs) -> double
                    {
                        return lhs + rhs;
                    }
            );
        }

        /* infix - */ {
            context.bind_op(
                    "-",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                    [](int64_t lhs, int64_t rhs) -> int64_t
                    {
                        return lhs - rhs;
                    }
            );

            context.bind_op(
                    "-",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                    [](uint64_t lhs, uint64_t rhs) -> uint64_t
                    {
                        return lhs - rhs;
                    }
            );

            context.bind_op(
                    "-",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                    [](double lhs, double rhs) -> double
                    {
                        return lhs - rhs;
                    }
            );
        }

        // logic op

        /* infix > */ {
            context.bind_op(
                    ">",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](int64_t lhs, int64_t rhs) -> bool
                    {
                        return lhs > rhs;
                    }
            );

            context.bind_op(
                    ">",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](uint64_t lhs, uint64_t rhs) -> bool
                    {
                        return lhs > rhs;
                    }
            );

            context.bind_op(
                    ">",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](double lhs, double rhs) -> bool
                    {
                        return lhs > rhs;
                    }
            );
        }

        /* infix >= */ {
            context.bind_op(
                    ">=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](int64_t lhs, int64_t rhs) -> bool
                    {
                        return lhs >= rhs;
                    }
            );

            context.bind_op(
                    ">=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](uint64_t lhs, uint64_t rhs) -> bool
                    {
                        return lhs >= rhs;
                    }
            );

            context.bind_op(
                    ">=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](double lhs, double rhs) -> bool
                    {
                        return lhs >= rhs;
                    }
            );
        }

        /* infix < */ {
            context.bind_op(
                    "<",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](int64_t lhs, int64_t rhs) -> bool
                    {
                        return lhs < rhs;
                    }
            );

            context.bind_op(
                    "<",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](uint64_t lhs, uint64_t rhs) -> bool
                    {
                        return lhs < rhs;
                    }
            );

            context.bind_op(
                    "<",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](double lhs, double rhs) -> bool
                    {
                        return lhs < rhs;
                    }
            );
        }

        /* infix <= */ {
            context.bind_op(
                    "<=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](int64_t lhs, int64_t rhs) -> bool
                    {
                        return lhs <= rhs;
                    }
            );

            context.bind_op(
                    "<=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](uint64_t lhs, uint64_t rhs) -> bool
                    {
                        return lhs <= rhs;
                    }
            );

            context.bind_op(
                    "<=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                    [](double lhs, double rhs) -> bool
                    {
                        return lhs <= rhs;
                    }
            );
        }

        /* infix == */ {
            context.bind_op(
                    "==",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                    [](int64_t lhs, int64_t rhs) -> bool
                    {
                        return lhs == rhs;
                    }
            );

            context.bind_op(
                    "==",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                    [](uint64_t lhs, uint64_t rhs) -> bool
                    {
                        return lhs == rhs;
                    }
            );

            context.bind_op(
                    "==",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                    [](double lhs, double rhs) -> bool
                    {
                        return lhs == rhs;
                    }
            );
        }

        /* infix != */ {
            context.bind_op(
                    "!=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                    [](int64_t lhs, int64_t rhs) -> bool
                    {
                        return lhs != rhs;
                    }
            );

            context.bind_op(
                    "!=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                    [](uint64_t lhs, uint64_t rhs) -> bool
                    {
                        return lhs != rhs;
                    }
            );

            context.bind_op(
                    "!=",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                    [](double lhs, double rhs) -> bool
                    {
                        return lhs != rhs;
                    }
            );
        }

        // "&"  80
        // "^"  90
        // "|"  100

        /* infix && */ {
            context.bind_op(
                    "&&",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 110},
                    [](bool lhs, bool rhs) -> bool
                    {
                        return lhs && rhs;
                    }
            );
        }

        /* infix || */ {
            context.bind_op(
                    "||",
                    {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 120},
                    [](bool lhs, bool rhs) -> bool
                    {
                        return lhs || rhs;
                    }
            );
        }

#if 1
        context.bind_func(
                "if",
                [](std::vector<std::any> args) -> std::any
                {
                    using Result = runtime::result;

                    if (args.size() == 3)
                    {
                        if (std::any_cast<bool>(args[0]) != false)
                        {
                            return args[1];
                        }
                        else
                        {
                            return args[2];
                        }
                    }
                    else
                    {
                        throw std::invalid_argument("core::if requires 3 arguments, but "s + std::to_string(args.size()) + " passed!");
                    }
                }
        );
#endif
        // boolean constants

        context.bind_var("true", true);
        context.bind_var("false", false);

        return context;
    }
};

BOOST_FIXTURE_TEST_SUITE(s, F)

BOOST_AUTO_TEST_CASE(eval_lexemes)
{
    using namespace std::string_literals;

    std::string input {"1"};

    auto result = ctx.eval(input);
    BOOST_TEST(result.has_value());
    if (auto val = result.get_if<double>())
    {
        BOOST_TEST((1.0 == *val));
        BOOST_TEST((result == 1.0));
        BOOST_TEST((1.0 == ctx.eval(input).as<double>()));
    }

    std::string str {"\"hello, kaleidoscope!\""};

    auto result2 = ctx.eval(str);
    BOOST_TEST(result2.has_value());
    BOOST_TEST((*result2.get_if<std::string>() == "hello, kaleidoscope!"s));
}

BOOST_AUTO_TEST_CASE(eval_operators)
{
    BOOST_REQUIRE_THROW(ctx.eval("1 =%= 1"), std::runtime_error); // unknown operator

    BOOST_TEST((ctx.eval("-1") == -1LL)); // FIXME: parse error
    BOOST_TEST((ctx.eval("+1") == +1LL)); // FIXME: parse error
    BOOST_TEST((ctx.eval("!0") == !0LL)); // FIXME: parse error
    BOOST_TEST((ctx.eval("!1") == !1LL)); // FIXME: parse error

    BOOST_TEST((ctx.eval("1 + 1") == 2LL));
}

BOOST_AUTO_TEST_CASE(declare_variables)
{
    ctx.push_scope();
    {
        ctx.eval("var num: f64 = 5.0");
        BOOST_TEST((ctx.eval("num") == 5.0));

        ctx.eval("var num_2: f64");
        BOOST_TEST((ctx.eval("num_2") == 0.0));
    }
    ctx.pop_scope();
}

BOOST_AUTO_TEST_CASE(bind_callable)
{
    // function foo(a: f64): f64;
    ctx["foo"] = [](double a) -> double
    {
        return a * a;
    };
    // function foo(a: i32): i32;
    ctx["foo"] = [](int64_t a) -> int64_t
    {
        return a * 2;
    };

    BOOST_TEST((ctx.eval("foo(4.5)") == (4.5 * 4.5)));
    BOOST_TEST((ctx.eval("foo(4)") == 8LL));
}

BOOST_AUTO_TEST_SUITE_END()
