#include <kaleidoscope/state.hpp>

#define BOOST_TEST_MODULE kaleidoscope_context
#include <boost/test/included/unit_test.hpp>

struct Fixture
{
    kaleidoscope::state ctx;
};

BOOST_FIXTURE_TEST_SUITE(BOOST_TEST_MODULE, Fixture)

BOOST_AUTO_TEST_CASE(unregistered_type)
{
    BOOST_CHECK_THROW(ctx.eval("5"), std::runtime_error); // неизвестно преобразование из литерала в тип интерпретатора

    ctx.register_type<int32_t>("i32"); // регистрация разрешенного типа данных

    // явный конвертер литерала числа в зарегистрированный числовой тип
    ctx.register_literal_promotion<kaleidoscope::ast::Literal_Numeric>(
            [](const kaleidoscope::ast::Literal_Numeric &node) -> kaleidoscope::runtime::result
            {
                return std::visit([](const auto &v) -> kaleidoscope::runtime::result
                                  {
                                      return static_cast<int32_t>(v);
                                  },
                                  node.value);
            }
    );

    BOOST_TEST((ctx.eval("5") == static_cast<int32_t>(5))); // явная проверка

    ctx.push_scope(); // открыть новую локальную область видимости
    {
        ctx.eval("var x: i32"); // локальная переменная
        BOOST_TEST((ctx.eval("x") == static_cast<int32_t>(0))); // никакого в неинициализированных переменных
    }
    ctx.pop_scope(); // закрыть локальную область видимости, все данные уничтожаются и больше недоступны
}

BOOST_AUTO_TEST_SUITE_END()
