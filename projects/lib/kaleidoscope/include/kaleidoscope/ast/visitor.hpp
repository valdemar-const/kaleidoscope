#pragma once

#include <kaleidoscope/ast.hpp>

#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <concepts>
#include <stdexcept>

namespace kaleidoscope::ast::utils
{

template<typename V, typename Base>
struct Visitor_Node_CRTP
{
    using Handler = std::function<void(const Base &)>;

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
        handlers.emplace(typeid(T), [f = std::forward<F>(func)](const Base &obj)
                         {
                             f(*static_cast<const T *>(&obj));
                         });
    }

    template<typename T>
    V &
    visit(const T &obj)
    {
        const auto &id = typeid(obj);
        if (handlers.count(id))
        {
            handlers.at(id)(obj);
        }
        else if (!is_ignore_unhandled_)
        {
            throw std::runtime_error(std::string("unregistered hundle for ") + id.name());
        }
        else
        {
            // do nothing
        }

        return *static_cast<V *>(this);
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
