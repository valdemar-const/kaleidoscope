#pragma once

#include <entt/entt.hpp>

#include <anyany/anyany.hpp>
#include <anyany/anyany_macro.hpp>

#include <boost/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

#include <string>
#include <functional>
#include <optional>
#include <any>
#include <memory>
#include <variant>
#include <unordered_map>
#include <typeindex>

#include <compiler/fix_interface_definition.hpp>

/** uuid component */
namespace kaleidoscope::alt::ecs::component
{
using uuid = boost::uuids::uuid;
} // namespace kaleidoscope::alt::ecs::component

/** declaration kaleidoscope::alt::Context::Entity */
namespace kaleidoscope::alt
{

struct Entity
{
    Entity(entt::registry &owner, entt::entity id)
        : id_(id)
        , owner_(&owner)
    {
    }

    Entity(const Entity &)            = default;
    Entity(Entity &&)                 = default;
    Entity &operator=(const Entity &) = default;
    Entity &operator=(Entity &&)      = default;

    operator entt::entity(void) const
    {
        return id_;
    }

  protected:

    entt::entity    id_;
    entt::registry *owner_;
};

}; // namespace kaleidoscope::alt

/** category: AST container */
namespace kaleidoscope::alt::category
{

struct Ast
{
};

} // namespace kaleidoscope::alt::category

/** category: AST nodes */
namespace kaleidoscope::alt::category::ast
{

struct NumericLexeme
{
};

}; // namespace kaleidoscope::alt::category::ast

/** components: AST nodes */
namespace kaleidoscope::alt::ast
{

struct NumericLexeme
{
    using Value = std::variant<int64_t, uint64_t, float, double>;

    Value value = static_cast<int64_t>(0);
};

}; // namespace kaleidoscope::alt::ast

/** components: AST container */
namespace kaleidoscope::alt
{

struct Ast
{
    Ast(Entity root)
        : root(root)
    {
    }

    Ast(const Ast &)            = default;
    Ast(Ast &&)                 = default;
    Ast &operator=(const Ast &) = default;
    Ast &operator=(Ast &&)      = default;

    Entity root;
};

} // namespace kaleidoscope::alt

/** generic category factory */
namespace kaleidoscope::alt::ecs
{

template<typename Derrived>
struct FactoryCRTP
{
    template<typename... Args>
    Entity &
    build(entt::registry &registry, Entity &entity, Args... args)
    {
        return static_cast<Derrived *>(this)->build(registry, entity, std::forward<Args>(args)...);
    }
};

template<typename Category>
struct Factory : public FactoryCRTP<Factory<Category>>
{
    using category_t = Category;

    template<typename... Args>
    Entity &
    build(entt::registry &registry, Entity &entity, Args... args)
    {
        throw std::runtime_error(std::string {"Unimplemented Category factory, where Category is "} + typeid(category_t).name() + ".");
    }
}; // namespace kaleidoscope::alt::ecs

} // namespace kaleidoscope::alt::ecs

/** generic interface adapter */
namespace kaleidoscope::alt::ecs
{

template<typename Derrived>
struct AdapterCRTP
{
    template<typename... Args>
    Entity &
    make_proxy(entt::registry &registry, Entity &entity, Args... args)
    {
        return static_cast<Derrived *>(this)->make_proxy(registry, entity, std::forward<Args>(args)...);
    }
};

template<typename Interface>
struct Adapter : public AdapterCRTP<Adapter<Interface>>
{
    using interface_t = Interface;

    template<typename... Args>
    Entity &
    build(entt::registry &registry, Entity &entity, Args... args)
    {
        throw std::runtime_error(std::string {"Unimplemented Category factory, where Category is "} + typeid(interface_t).name() + ".");
    }
}; // namespace kaleidoscope::alt::ecs

} // namespace kaleidoscope::alt::ecs

/** category factory specialization for <category::ast::NumericLexeme> */
namespace kaleidoscope::alt::ecs
{

template<>
struct Factory<category::ast::NumericLexeme> : public FactoryCRTP<Factory<category::ast::NumericLexeme>>
{
    Factory(std::function<boost::uuids::uuid(void)> gen)
        : gen_(gen)
    {
    }

    template<typename... Args>
    Entity &
    build(entt::registry &registry, Entity &entity, Args... args)
    {
        entt::entity e = entity;
        registry.emplace_or_replace<boost::uuids::uuid>(e, gen_());
        registry.emplace_or_replace<alt::ast::NumericLexeme>(e, std::forward<Args>(args)...);

        return entity;
    }

  protected:

    std::function<boost::uuids::uuid(void)> gen_;
};

} // namespace kaleidoscope::alt::ecs

/** category factory specialization for <category::Ast> */
namespace kaleidoscope::alt::ecs
{

template<>
struct Factory<category::Ast> : public FactoryCRTP<Factory<category::Ast>>
{
    Factory(std::function<boost::uuids::uuid(void)> gen)
        : gen_(gen)
    {
    }

    template<typename... Args>
    Entity &
    build(entt::registry &registry, Entity &entity, Args... args)
    {
        entt::entity e = entity;
        registry.emplace_or_replace<boost::uuids::uuid>(e, gen_());
        registry.emplace_or_replace<alt::Ast>(e, std::forward<Args>(args)...);

        return entity;
    }

  protected:

    std::function<boost::uuids::uuid(void)> gen_;
};

} // namespace kaleidoscope::alt::ecs

/** category container for registry */
namespace kaleidoscope::alt::ecs::component
{

struct EntityCategoryFactory
{
    using Factory = std::any;

    std::unordered_map<std::type_index, Factory> storage;
};

}; // namespace kaleidoscope::alt::ecs::component

/** interface adapter container for registry */
namespace kaleidoscope::alt::ecs::component
{

struct EntityInterfaceAdapter
{
    using Adapter = std::any;

    std::unordered_map<std::type_index, Adapter> storage;
};

}; // namespace kaleidoscope::alt::ecs::component

/** anyany methods */
namespace kaleidoscope::alt::interface::method
{
anyany_method(get_uuid, (&self) requires(self.get_id())->boost::uuids::uuid);
} // namespace kaleidoscope::alt::interface::method

/** interfaces */
namespace kaleidoscope::alt::interface
{
using namespace method;

using IAstNode = aa::any_with<get_uuid>;
using IAst     = aa::any_with<get_uuid>;
} // namespace kaleidoscope::alt::interface

/** interface impl */
namespace kaleidoscope::alt::proxy
{

struct Ast
{
    Ast(entt::registry &owner, entt::entity entity)
        : owner_(&owner)
        , entity_(entity)
    {
    }

    Ast(const Ast &)            = default;
    Ast(Ast &&)                 = default;
    Ast &operator=(const Ast &) = default;
    Ast &operator=(Ast &&)      = default;

  public:

    ecs::component::uuid
    get_id(void)
    {
        return owner_->get<ecs::component::uuid>(entity_);
    }

  protected:

    entt::registry *owner_;
    entt::entity    entity_;
};

} // namespace kaleidoscope::alt::proxy

/** interface adapter specialization */
namespace kaleidoscope::alt::ecs
{

template<>
struct Adapter<interface::IAst> : public AdapterCRTP<Adapter<interface::IAst>>
{
    Adapter(std::function<boost::uuids::uuid(void)> gen)
        : gen_(gen)
    {
    }

    template<typename... Args>
    std::optional<interface::IAst>
    make_proxy(entt::registry &registry, Entity &entity, Args... args)
    {
        entt::entity e = entity;

        if (registry.all_of<ecs::component::uuid, alt::Ast>(e))
        {
            return proxy::Ast {registry, e};
        }

        return std::nullopt;
    }

  protected:

    std::function<boost::uuids::uuid(void)> gen_;
};

} // namespace kaleidoscope::alt::ecs

/** declaration kaleidoscope::alt::Context */
namespace kaleidoscope::alt
{

struct Context
{
    Context(void);

  public:

    template<typename Category, typename Factory>
    void register_category(Factory &&impl);

    template<typename Category, typename Adapter>
    void register_interface_adapter(Adapter &&impl);

  public:

    template<typename Category, typename... Args>
    std::optional<Entity>
    create(Args... args);

    template<typename Interface, typename... Args>
    std::optional<Interface>
    as_interface(Entity entity, Args... args);

  protected:

    entt::registry reg_;
};

} // namespace kaleidoscope::alt

/** definition kaleidoscope::alt::Context */
namespace kaleidoscope::alt
{

Context::Context(void)
{
    reg_.ctx().emplace<ecs::component::EntityCategoryFactory>();
    reg_.ctx().emplace<ecs::component::EntityInterfaceAdapter>();
}

template<typename Category, typename Factory>
inline void
Context::register_category(Factory &&impl)
{
    auto &storage = reg_.ctx().get<ecs::component::EntityCategoryFactory>().storage;

    if (!storage.contains(typeid(Category)))
    {
        storage.emplace(typeid(Category), std::move(impl));
    }
}

template<typename Category, typename Adapter>
inline void
Context::register_interface_adapter(Adapter &&impl)
{
    auto &storage = reg_.ctx().get<ecs::component::EntityInterfaceAdapter>().storage;

    if (!storage.contains(typeid(Category)))
    {
        storage.emplace(typeid(Category), std::move(impl));
    }
}

template<typename Category, typename... Args>
inline std::optional<Entity>
Context::create(Args... args)
{
    auto &storage = reg_.ctx().get<ecs::component::EntityCategoryFactory>().storage;

    if (storage.contains(typeid(Category)))
    {
        auto  &factory = *std::any_cast<ecs::Factory<Category>>(&storage.at(typeid(Category)));
        Entity entity {reg_, reg_.create()};

        return factory.build(reg_, entity, std::forward<Args>(args)...);
    }

    return std::nullopt;
}

template<typename Interface, typename... Args>
inline std::optional<Interface>
Context::as_interface(Entity entity, Args... args)
{
    auto &storage = reg_.ctx().get<ecs::component::EntityInterfaceAdapter>().storage;

    if (storage.contains(typeid(Interface)))
    {
        auto &adapter = *std::any_cast<ecs::Adapter<Interface>>(&storage.at(typeid(Interface)));

        return adapter.make_proxy(reg_, entity, std::forward<Args>(args)...);
    }

    return std::nullopt;
}

} // namespace kaleidoscope::alt
