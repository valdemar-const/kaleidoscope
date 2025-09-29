#include <kaleidoscope/alt/context.hpp>

#define BOOST_TEST_MODULE kaleidoscope_parser
#include <boost/test/included/unit_test.hpp>

#undef interface

using namespace std::string_literals;

struct F
{
  public:

    F(void)
    {
        BOOST_TEST_MESSAGE("setup fixture");
    }

    ~F(void)
    {
        BOOST_TEST_MESSAGE("teardown fixture");
    }

    kaleidoscope::alt::Context context;
};

BOOST_FIXTURE_TEST_SUITE(s, F)

BOOST_AUTO_TEST_CASE(parse_numeric_lexeme)
{
    using namespace kaleidoscope::alt;

    auto entity = context.create<category::ast::NumericLexeme>();
    BOOST_TEST(!entity);

    context.register_category<category::ast::NumericLexeme>(
            ecs::Factory<category::ast::NumericLexeme> {boost::uuids::random_generator_mt19937 {}}
    );

    auto number = context.create<category::ast::NumericLexeme>();
    BOOST_TEST(number.has_value());

    context.register_category<category::Ast>(
            ecs::Factory<category::Ast> {boost::uuids::random_generator_mt19937 {}}
    );

    auto ast = context.create<category::Ast>(number.value());

    if (auto ast_proxy_opt = context.as_interface<interface::IAst>(ast.value()))
    {
        auto &ast = *ast_proxy_opt;

        BOOST_TEST_MESSAGE(ast.get_uuid());
    }
}

BOOST_AUTO_TEST_SUITE_END()
