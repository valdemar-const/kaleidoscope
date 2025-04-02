#pragma once

#include <kaleidoscope/ast.hpp>

#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>

namespace kaleidoscope::ast::utils
{

template<typename V, typename Base>
struct Visitor_Node_CRTP
{
    using Handler = std::function<void(const Base &)>;

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
        return *static_cast<V *>(this);
    }

  protected:

    std::unordered_map<std::type_index, Handler> handlers;
};

} // namespace kaleidoscope::ast::utils
