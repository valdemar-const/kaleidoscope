#pragma once

#include <kaleidoscope/type_info.hpp>

#include <string>
#include <string_view>
#include <typeindex>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <stack>
#include <map>
#include <unordered_set>
#include <any>
#include <iterator>

namespace kaleidoscope::type
{

struct Get_Children
{
    using iterator_category = std::forward_iterator_tag;
    using value_type        = type::Info;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type *;
    using reference         = value_type &;

  public:

    std::vector<type::Info *> operator()(const type::Info &node) const;

  private:

    static std::vector<type::Info *> get_childs(const type::Void &node);
    static std::vector<type::Info *> get_childs(const type::Primitive &node);
    static std::vector<type::Info *> get_childs(const type::Formal_Parameter &node);
    static std::vector<type::Info *> get_childs(const type::Function &node);
    static std::vector<type::Info *> get_childs(const type::Variable &node);
    static std::vector<type::Info *> get_childs(const type::Mutable_Qualifier &node);
    static std::vector<type::Info *> get_childs(const type::Operator &node);
    static std::vector<type::Info *> get_childs(const type::Cpp_Type &node);

    template<typename T>
        requires(std::is_base_of_v<type::Info, std::decay_t<T>>)
    static std::vector<type::Info *> get_childs(const type::Info *any_node);
};

} // namespace kaleidoscope::type

// inl kaleidoscope::type::Get_Children
namespace kaleidoscope::type
{

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Void &node)
{
    return {};
}

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Primitive &node)
{
    return {};
}

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Formal_Parameter &node)
{
    return {};
}

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Function &node)
{
    return {};
}

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Variable &node)
{
    return {};
}

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Mutable_Qualifier &node)
{
    return {node.redirect.get()};
}

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Operator &node)
{
    return {};
}

inline std::vector<type::Info *>
Get_Children::get_childs(const type::Cpp_Type &node)
{
    return {node.inner.get()};
}

template<typename T>
    requires(std::is_base_of_v<type::Info, std::decay_t<T>>)
inline std::vector<type::Info *>
Get_Children::get_childs(const type::Info *any_node)
{
    using Ptr = const std::decay_t<T> *;
    return get_childs(*dynamic_cast<Ptr>(any_node));
}

inline std::vector<type::Info *>
Get_Children::operator()(const type::Info &node) const
{
    std::type_index node_id {typeid(node)};

    static const std::unordered_map<std::type_index, std::function<std::vector<type::Info *>(const type::Info *)>> type_handler {
            // clang-format off
            std::make_pair(typeid(type::Void),              [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Void>             (any_node); }),
            std::make_pair(typeid(type::Primitive),         [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Primitive>        (any_node); }),
            std::make_pair(typeid(type::Formal_Parameter),  [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Formal_Parameter> (any_node); }),
            std::make_pair(typeid(type::Function),          [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Function>         (any_node); }),
            std::make_pair(typeid(type::Variable),          [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Variable>         (any_node); }),
            std::make_pair(typeid(type::Mutable_Qualifier), [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Mutable_Qualifier>(any_node); }),
            std::make_pair(typeid(type::Operator),          [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Operator>         (any_node); }),
            std::make_pair(typeid(type::Cpp_Type),          [](const type::Info *any_node) -> std::vector<type::Info *> { return get_childs<type::Cpp_Type>         (any_node); })
            // clang-format on
    };

    return type_handler.at(node_id)(&node);
}

} // namespace kaleidoscope::type

namespace kaleidoscope::type
{

class Children_Iterator
{
  public:

    using iterator_category = std::forward_iterator_tag;
    using value_type        = Info;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type *;
    using reference         = value_type &;

    explicit Children_Iterator(Info *parent)
        : children_(Get_Children {}(*parent))
        , idx_(0)
    {
    }

    reference
    operator*() const
    {
        return *children_[idx_];
    }

    pointer
    operator->() const
    {
        return children_[idx_];
    }

    Children_Iterator &
    operator++()
    {
        ++idx_;
        return *this;
    }

    Children_Iterator
    operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    bool
    operator==(const Children_Iterator &other) const
    {
        return children_ == other.children_ && idx_ == other.idx_;
    }

    bool
    operator!=(const Children_Iterator &other) const
    {
        return !(*this == other);
    }

    // конец - индекс за пределами вектора
    Children_Iterator
    end() const
    {
        auto it = *this;
        it.idx_ = children_.size();
        return it;
    }

  private:

    std::vector<Info *> children_;
    std::size_t         idx_;
};

} // namespace kaleidoscope::type

namespace kaleidoscope::type
{

class Children_View
{
    using Node = type::Info;

  public:

    using iterator       = Children_Iterator;
    using const_iterator = iterator;

    explicit Children_View(Node &n)
        : node_(&n)
    {
    }

    iterator
    begin() const
    {
        return iterator(node_);
    }

    iterator
    end() const
    {
        return iterator(node_).end();
    }

  private:

    Node *node_;
};

} // namespace kaleidoscope::type

namespace kaleidoscope::type
{

class Recursive_Iterator
{
    using Node = type::Info;

  public:

    using value_type        = Node;
    using reference         = Node &;
    using pointer           = Node *;
    using iterator_category = std::forward_iterator_tag;

    explicit Recursive_Iterator(Node *root = nullptr)
    {
        if (root)
        {
            push_frame(root);
        }
    }

    reference
    operator*() const
    {
        return *stack_.top().current;
    }

    pointer
    operator->() const
    {
        return &*stack_.top().current;
    }

    Recursive_Iterator &
    operator++()
    {
        while (!stack_.empty())
        {
            auto &frame = stack_.top();
            ++frame.current;

            if (frame.current != frame.end)
            {
                // Переходим к детям текущего узла
                push_frame(&(*frame.current));
                break;
            }

            // Уровень закончился - поднимаемся
            stack_.pop();
        }
        return *this;
    }

    Recursive_Iterator
    operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    bool
    operator==(const Recursive_Iterator &other) const
    {
        if (stack_.empty() && other.stack_.empty())
        {
            return true;
        }
        if (stack_.size() != other.stack_.size())
        {
            return false;
        }
        return stack_.top().current == other.stack_.top().current;
    }

    bool
    operator!=(const Recursive_Iterator &other) const
    {
        return !(*this == other);
    }

  private:

    struct Frame
    {
        Children_View     view;
        Children_Iterator current;
        Children_Iterator end;

        Frame(Node *node)
            : view(*node)
            , current(view.begin())
            , end(view.end())
        {
        }
    };

    std::stack<Frame> stack_;

    void
    push_frame(Node *n)
    {
        stack_.push(Frame(n));
    }
};

} // namespace kaleidoscope::type

namespace kaleidoscope::type
{

class Recursive_View
{
    using Node = type::Info;

  public:

    explicit Recursive_View(Node &root)
        : root_(&root)
    {
    }

    Recursive_Iterator
    begin() const
    {
        return Recursive_Iterator(root_);
    }

    Recursive_Iterator
    end() const
    {
        return Recursive_Iterator();
    }

  private:

    Node *root_;
};

} // namespace kaleidoscope::type

namespace kaleidoscope::type
{

struct Registry
{
    using Type_Info = type::Info;
    using Type_Key  = size_t;
    using Type_Ptr  = Type_Info *;

    template<traits::Type_Basic_Scalar T>
    Type_Ptr
    register_type(std::string type_name)
    {
        using Type = std::decay_t<T>;
        auto info  = type::make_info<Type>(type_name);
        auto index = info->fingerprint();

        auto [it, is_emplaced] = emplace(std::make_pair(index, std::move(info)));

        if (is_emplaced)
        {
            type_key_by_name_.emplace(std::make_pair(type_name, index));
            binded_types_.emplace(std::make_pair(typeid(T), index));
        }

        return (is_emplaced) ? it->second.get() : nullptr;
    }

    Type_Ptr
    emplace(std::unique_ptr<Type_Info> info)
    {
        if (!info)
        {
            return nullptr;
        }

        // Рекурсивно проверяем и заменяем узлы на существующие
        auto processed_info = replace_with_existing(std::move(info));
        auto fingerprint    = processed_info->fingerprint();

        // Проверяем, существует ли уже такой узел
        auto existing_it = storage_.find(fingerprint);
        if (existing_it != storage_.end())
        {
            // Возвращаем существующий узел
            return existing_it->second.get();
        }

        // Сохраняем новый узел
        auto [it, is_emplaced] = storage_.emplace(fingerprint, std::move(processed_info));

        if (is_emplaced && it->second->name)
        {
            type_key_by_name_.emplace(*(it->second->name), fingerprint);
        }

        return is_emplaced ? it->second.get() : nullptr;
    }

    // Утилитарные методы для обхода
    Children_View
    children(Type_Info &type)
    {
        return Children_View(type);
    }

    Recursive_View
    recursive(Type_Info &type)
    {
        return Recursive_View(type);
    }

    Type_Ptr
    find(Type_Key fingerprint)
    {
        auto it = storage_.find(fingerprint);
        return it != storage_.end() ? it->second.get() : nullptr;
    }

    Type_Ptr
    find(const std::string &name)
    {
        auto it = type_key_by_name_.find(name);
        if (it != type_key_by_name_.end())
        {
            return find(it->second);
        }
        return nullptr;
    }

  private:

    std::unique_ptr<Type_Info>
    replace_with_existing(std::unique_ptr<Type_Info> node)
    {
        return node;
    }

    void
    process_children(Type_Info &node)
    {
        return;
    }

  protected:

    std::map<std::string, Type_Key>                type_key_by_name_;
    std::map<std::type_index, Type_Key>            binded_types_;
    std::map<Type_Key, std::unique_ptr<Type_Info>> storage_;
};

} // namespace kaleidoscope::type
