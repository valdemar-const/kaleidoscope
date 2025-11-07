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
        using Kind          = kaleidoscope::Module::operator_properties::Kind;
        using Associativity = kaleidoscope::Module::operator_properties::Associativity;

        ctx.register_type<bool>("bool");
        ctx.register_type<int64_t>("i64");
        ctx.register_type<uint64_t>("u64");
        ctx.register_type<double>("f64");

        Module context;
        // script::core

        // - unary ops:

        context.bind_op(
                "+",
                Module::Operator {
                        {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 1)
                            {
                                return +(std::any_cast<Result>(args[0]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`+` requires 1 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "-",
                Module::Operator {
                        {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 1)
                            {
                                return -(std::any_cast<Result>(args[0]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`-` requires 1 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "!",
                Module::Operator {
                        {.kind = Kind::Prefix, .associativity = Associativity::Left, .precedence = 20},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 1)
                            {
                                bool is_true = std::any_cast<Result>(args[0]).as<double>() != 0.0;

                                return static_cast<double>(!is_true);
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`!` requires 1 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        // '!'
        // '~'

        // - binary ops:

        context.bind_op(
                "*",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return std::any_cast<Result>(args[0]).as<double>()
                                     * std::any_cast<Result>(args[1]).as<double>();
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`*` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "/",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return std::any_cast<Result>(args[0]).as<double>()
                                     / std::any_cast<Result>(args[1]).as<double>();
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`/` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "%",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 30},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                auto lhs = std::any_cast<Result>(args[0]).as<double>();
                                auto rhs = std::any_cast<Result>(args[1]).as<double>();

                                return static_cast<double>(
                                        static_cast<int>(lhs) % static_cast<int>(rhs)
                                );
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`%` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "+",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return std::any_cast<Result>(args[0]).as<double>()
                                     + std::any_cast<Result>(args[1]).as<double>();
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`+` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "-",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 40},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return std::any_cast<Result>(args[0]).as<double>()
                                     - std::any_cast<Result>(args[1]).as<double>();
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`-` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        // logic op

        context.bind_op(
                ">",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                > std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`>` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                ">=",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                >= std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`>=` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "<",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                < std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`<` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "<=",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 60},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                <= std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`<=` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "==",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                == std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`==` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "!=",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 70},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                != std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`!=` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        // "&"  80
        // "^"  90
        // "|"  100

        context.bind_op(
                "&&",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 110},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                && std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`&&` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_op(
                "||",
                Module::Operator {
                        {.kind = Kind::Infix, .associativity = Associativity::Left, .precedence = 120},
                        [](std::vector<std::any> args) -> std::any
                        {
                            using Result = runtime::result;

                            if (args.size() == 2)
                            {
                                return (double)(std::any_cast<Result>(args[0]).as<double>()
                                                || std::any_cast<Result>(args[1]).as<double>());
                            }
                            else
                            {
                                throw std::invalid_argument("core::operator`||` requires 2 arguments, but "s + std::to_string(args.size()) + " passed!");
                            }
                        }
                }
        );

        context.bind_func(
                "if",
                [](std::vector<std::any> args) -> std::any
                {
                    using Result = runtime::result;

                    if (args.size() == 3)
                    {
                        constexpr auto False = 0.0;
                        if (std::any_cast<Result>(args[0]).as<double>() != False)
                        {
                            return std::any_cast<Result>(args[1]).as<double>();
                        }
                        else
                        {
                            return std::any_cast<Result>(args[2]).as<double>();
                        }
                    }
                    else
                    {
                        throw std::invalid_argument("core::if requires 3 arguments, but "s + std::to_string(args.size()) + " passed!");
                    }
                }
        );

        // boolean constants

        context.bind_var("true", 1.0);
        context.bind_var("false", 0.0);

        return context;
    }
};

BOOST_FIXTURE_TEST_SUITE(s, F)

BOOST_AUTO_TEST_CASE(eval_lexemes)
{
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
    BOOST_TEST((result2 == "hello, kaleidoscope!"));
}

BOOST_AUTO_TEST_CASE(eval_operators)
{
    BOOST_REQUIRE_THROW(ctx.eval("1 =%= 1"), std::runtime_error); // unknown operator

    BOOST_TEST((ctx.eval("-1") == -1.0)); // FIXME: parse error
    BOOST_TEST((ctx.eval("+1") == +1.0)); // FIXME: parse error
    BOOST_TEST((ctx.eval("!0") == 1.0));  // FIXME: parse error
    BOOST_TEST((ctx.eval("!1") == 0.0));  // FIXME: parse error

    BOOST_TEST((ctx.eval("1 + 1") == 2.0));
}

BOOST_AUTO_TEST_CASE(bind_callable)
{
    // function foo(a: f64): f64;
    ctx["foo"] = [](double a) -> double
    {
        return a * a;
    };
    // function foo(a: i32): i32;
    ctx["foo"] = [](int a) -> int
    {
        return a * 2;
    };

    BOOST_TEST((ctx.eval("foo(4.0)") == 16.0));
    BOOST_TEST((ctx.eval("foo(4)") == 8));
}

BOOST_AUTO_TEST_SUITE_END()
