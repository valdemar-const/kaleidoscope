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

namespace kaleidoscope::alt
{
struct Entity; // forward decl
} // namespace kaleidoscope::alt

namespace kaleidoscope::alt::category::ast
{

struct NumericLexeme
{
};

}; // namespace kaleidoscope::alt::category::ast

namespace kaleidoscope::alt::ast
{

struct NumericLexeme
{
    using Value = std::variant<int64_t, uint64_t, float, double>;

    Value value = static_cast<int64_t>(0);
};

}; // namespace kaleidoscope::alt::ast

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

namespace kaleidoscope::alt::ecs::component
{

struct EntityCategoryFactory
{
    using Factory = std::any;

    std::unordered_map<std::type_index, Factory> storage;
};

}; // namespace kaleidoscope::alt::ecs::component

namespace kaleidoscope::alt::interface::method
{
anyany_method(get_uuid, (&self) requires(self.get_id())->boost::uuids::uuid);
} // namespace kaleidoscope::alt::interface::method

namespace kaleidoscope::alt::interface
{
using namespace method;

using IAstNode = aa::any_with<get_uuid>;
} // namespace kaleidoscope::alt::interface

// declaration kaleidoscope::alt::Context
namespace kaleidoscope::alt
{

struct Context
{
    Context(void);

  public:

    template<typename Category, typename... Args>
    std::optional<Entity>
    create(Args... args);

    template<typename Category, typename Factory>
    void register_category(Factory &&impl);

  protected:

    entt::registry reg_;
};

} // namespace kaleidoscope::alt

// declaration kaleidoscope::alt::Context::Entity
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

namespace kaleidoscope::alt
{

Context::Context(void)
{
    reg_.ctx().emplace<ecs::component::EntityCategoryFactory>();
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

} // namespace kaleidoscope::alt
