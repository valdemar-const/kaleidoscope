#pragma once

#include <kaleidoscope/ast.hpp>

#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <concepts>
#include <stdexcept>
#include <optional>

namespace kaleidoscope::ast::utils
{

template<typename V, typename Base, typename ResultType = void>
struct Visitor_Node_CRTP
{
    using result_t                         = std::decay_t<ResultType>;
    using Handler                          = std::function<result_t(const Base &)>;
    static constexpr bool has_return_value = !std::is_void_v<result_t>;
    using Result                           = std::conditional_t<std::is_void_v<result_t>, V&, std::optional<result_t>>;

  protected:

    template<typename T>
    struct overload
    {
        using value = void (V::*)(const T &);

        template<typename F>
        constexpr value
        get(F &&method)
        {
            return static_cast<value>(method);
        }
    };

  public:

    Visitor_Node_CRTP(bool ignore_unhandled = true)
        : is_ignore_unhandled_(ignore_unhandled)
    {
    }

  public:

    template<typename T, typename F>
    void
    register_handler(F &&func)
    {
        handlers.emplace(typeid(T), [f = std::forward<F>(func)](const Base &obj) -> result_t
                         {
                             return f(*static_cast<const T *>(&obj));
                         });
    }

    template<typename T>
    Result
    visit(const T &obj)
    {
        const auto &id = typeid(obj);
        if (handlers.count(id))
        {
            if constexpr (has_return_value)
            {
                return std::make_optional(handlers.at(id)(obj));
            }
            else
            {
                handlers.at(id)(obj);
                return *static_cast<V *>(this);
            }
        }
        else if (!is_ignore_unhandled_)
        {
            throw std::runtime_error(std::string("unregistered hundle for ") + id.name());
        }
        else
        {
            if constexpr (has_return_value)
            {
                return std::nullopt;
            }
            else
            {
                return *static_cast<V *>(this);
            }
        }
    }

  protected:

    template<typename T, typename F>
    void
    register_method_handler(F &&method)
    {
        register_handler<T>(std::bind(method, static_cast<V *>(this), std::placeholders::_1));
    }

  protected:

    std::unordered_map<std::type_index, Handler> handlers;
    bool                                         is_ignore_unhandled_;
};

} // namespace kaleidoscope::ast::utils
