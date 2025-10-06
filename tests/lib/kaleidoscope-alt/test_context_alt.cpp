#include <kaleidoscope/alt/context.hpp>

#define BOOST_TEST_MODULE kaleidoscope_parser
#include <boost/test/included/unit_test.hpp>

#include <compiler/fix_interface_definition.hpp>

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

    kaleidoscope::alt::Context             context;
    boost::uuids::random_generator_mt19937 get_uuid {};
};

BOOST_FIXTURE_TEST_SUITE(s, F)

BOOST_AUTO_TEST_CASE(parse_numeric_lexeme)
{
    using namespace kaleidoscope::alt;

    auto entity = context.create<category::ast::NumericLexeme>();
    BOOST_TEST(!entity);

    context.register_category<category::ast::NumericLexeme>(
            ecs::Factory<category::ast::NumericLexeme> {get_uuid}
    );

    auto number = context.create<category::ast::NumericLexeme>();
    BOOST_TEST(number.has_value());

    context.register_category<category::Ast>(
            ecs::Factory<category::Ast> {get_uuid}
    );

    auto ast = context.create<category::Ast>(number.value());

    BOOST_TEST(!context.as_interface<interface::IAst>(ast.value()));

    context.register_interface_adapter<interface::IAst>(ecs::Adapter<interface::IAst> {get_uuid});

    if (auto ast_proxy_opt = context.as_interface<interface::IAst>(ast.value()))
    {
        auto &ast = *ast_proxy_opt;

        std::stringstream msg;
        msg << "{type: \"Ast\", uuid: \"{" << ast.get_uuid() << "}\"}";
        BOOST_TEST_MESSAGE(msg.str());
    }
}

BOOST_AUTO_TEST_SUITE_END()
