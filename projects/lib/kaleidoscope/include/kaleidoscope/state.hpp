#pragma once

#include <kaleidoscope/parser.hpp>
#include <kaleidoscope/parser/support/from_string_view.hpp>
#include <kaleidoscope/ast/precedence.hpp>

#include <kaleidoscope/module.hpp>
#include <kaleidoscope/runtime.hpp>

#include <unordered_map>
#include <span>
#include <list>
#include <variant>
#include <type_traits>
#include <concepts>

#include <iostream>

namespace kaleidoscope
{

struct state
{
  public:

    using Cache_Ast           = std::unordered_map<std::string, Ast>;
    using Operator_Properties = Module::Operator_Properties;
    using result              = runtime::result;

    struct symbol
    {
        result
        operator()(void)
        {
            return {};
        }
    };

    struct Data_Binder
    {
        Data_Binder(state &owner, std::string symbol_name)
            : owner_(owner)
            , symbol_name_(symbol_name)
        {
        }

        Data_Binder(Data_Binder &)             = delete;
        Data_Binder(Data_Binder &&)            = default;
        Data_Binder &operator=(Data_Binder &)  = delete;
        Data_Binder &operator=(Data_Binder &&) = default;

        state &
        operator=(double value)
        {
            owner_.get().runtime_.scope().bind_func(
                    symbol_name_,
                    [value = value](std::vector<std::any> args) -> std::any
                    {
                        return value;
                    }
            );
            return owner_.get();
        }

        state &
        operator=(Module::Functional::type value)
        {
            owner_.get().runtime_.scope().bind_func(symbol_name_, value);
            return owner_.get();
        }

        template<traits::Type_Callable T>
        state &
        operator=(T &&value)
        {
            owner_.get().runtime_.scope().bind_func(symbol_name_, std::forward<T>(value));
            return owner_.get();
        }

        state &
        operator=(Module::Operator value)
        {
            owner_.get().runtime_.scope().bind_op(symbol_name_, value);
            return owner_.get();
        }

      protected:

        std::reference_wrapper<state> owner_;
        std::string                   symbol_name_;
    };

  public:

    state(void)
        : global_()
        , runtime_(global_)
    {
    }

  public:

    void
    import(Module &&module)
    {
        global_.link_static(module);
    }

    void
    import(const Module &module)
    {
        global_.link_shared(module);
    }

  public:

    runtime &
    get_rt(void)
    {
        return runtime_;
    }

    template<typename F>
    state &
    add_operator(std::string name, Operator_Properties props, F impl)
    {
        return *this;
    }

    Data_Binder
    operator[](std::string symbol)
    {
        return Data_Binder {*this, symbol};
    }

  public:

    template<typename T>
    state &
    register_type(std::string_view name)
    {
        runtime_.scope().bind_type<T>(name.data());

        return *this;
    }

  public:

    state &
    source(std::string_view src)
    {
        // формирование AST
        auto ast = preprocess(parser_.parse(src.begin(), src.end()));
        // применить определения переменных операторов и функций ast к текущему глобальному модулю/области видимости
        // выделить тело анонимной функции из ast (список выражений) и подготовить его для выполнения из operator()
        // результат функции -> результат последнего выражения
        state_body_ = std::move(apply(std::move(ast)));

        return *this;
    }

    result
    eval(std::string_view src)
    {
        if (!ast_cache_.contains(src.data()))
        {
            auto ast = preprocess(parser_.parse(src.begin(), src.end()));
            ast_cache_.insert(std::make_pair(src, std::move(ast)));
        }
        return runtime_.eval(global_, ast_cache_.at(src.data()));
    }

    state &
    push_scope(void)
    {
        runtime_.push_scope();
        return *this;
    }

    state &
    pop_scope(void)
    {
        runtime_.pop_scope();
        return *this;
    }

    Cache_Ast &
    ast_cache(void)
    {
        return ast_cache_;
    }

  public:

    result
    operator()(void)
    {
        return {};
    }

  protected:

    Ast
    preprocess(Ast ast)
    {
        auto operators_info = runtime_.scope().collect_operators_info();
        ast::utils::precedence {operators_info}(ast);
        return ast;
    }

    Ast
    apply(Ast ast)
    {
        // пробежаться по
        return ast;
    }

  protected:

    Parser parser_;

  protected:

    Module            global_;
    runtime           runtime_;
    Ast               state_body_;
    mutable Cache_Ast ast_cache_;
};
} // namespace kaleidoscope
