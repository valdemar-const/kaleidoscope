#pragma once

#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <bit>
#include <typeindex>
#include <type_traits>

#include <boost/uuid.hpp>
#include <boost/callable_traits.hpp>

#include <cinttypes>
#include <cstddef>

namespace kaleidoscope::type::memory
{

constexpr size_t
round_to_power_of_two(size_t value)
{
    if (value == 0)
    {
        return 1;
    }
    return std::bit_ceil(value);
}

struct Layout
{
    virtual ~Layout(void) = default;

    virtual size_t size(void) const      = 0;
    virtual size_t alignment(void) const = 0;
};

struct Primitive final : public Layout
{
    ~Primitive(void) override = default;

    Primitive(void)
        : size_(sizeof(std::byte))
        , alignment_(alignof(std::byte))
    {
    }

    Primitive(size_t size, size_t alignment)
        : size_(size)
        , alignment_(round_to_power_of_two(alignment))
    {
    }

    Primitive(const Primitive &)            = default;
    Primitive(Primitive &&)                 = default;
    Primitive &operator=(const Primitive &) = default;
    Primitive &operator=(Primitive &&)      = default;

  public:

    size_t
    size(void) const override
    {
        return size_;
    }

    size_t
    alignment(void) const override
    {
        return alignment_;
    }

  protected:

    size_t size_;
    size_t alignment_;
};

struct Array final : public Layout
{
    ~Array(void) override = default;

    Array(Layout *element, size_t size = 1)
        : element_(element)
        , size_(size)
    {
    }

    Array(const Array &)            = default;
    Array(Array &&)                 = default;
    Array &operator=(const Array &) = default;
    Array &operator=(Array &&)      = default;

  public:

    size_t
    size(void) const override
    {
        return (element_->size() * size_ + alignment() - 1) / alignment() * alignment();
    }

    size_t
    alignment(void) const override
    {
        return element_->alignment();
    }

  protected:

    Layout *element_;
    size_t  size_;
};

#if 0
struct Product final : public Layout
{
    ~Product(void) override = default;

    Product(void) = default;

    Product(std::initializer_list<Layout *> fields)
        : fields_(std::move(fields))
    {
    }

    Product(const Product &)            = default;
    Product(Product &&)                 = default;
    Product &operator=(const Product &) = default;
    Product &operator=(Product &&)      = default;

  public:

    Product &
    add_field(Layout *field)
    {
        fields_.emplace_back(field);

        return *this;
    }

  public:

    size_t
    size(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().size();
        }

        return size_; // TODO: calculate from fields
    }

    size_t
    alignment(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().alignment();
        }

        return alignment_; // TODO: calculate from fields
    }

  protected:

    std::vector<Layout *> fields_;

  private:

    static Primitive &
    empty_layout(void)
    {
        static Primitive empty_layout_ {sizeof(std::byte), alignof(std::byte)};
        return empty_layout_;
    }
};

struct Sum final : public Layout
{
    ~Sum(void) override = default;

    Sum(void) = default;

    Sum(std::initializer_list<Layout *> fields)
        : fields_(std::move(fields))
    {
    }

    Sum(const Sum &)            = default;
    Sum(Sum &&)                 = default;
    Sum &operator=(const Sum &) = default;
    Sum &operator=(Sum &&)      = default;

  public:

    Sum &
    add_field(Layout *field)
    {
        fields_.emplace_back(field);

        return *this;
    }

  public:

    size_t
    size(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().size();
        }

        return size_; // TODO: calculate from fields
    }

    size_t
    alignment(void) override
    {
        if (fields_.empty())
        {
            return empty_layout().alignment();
        }

        return alignment_; // TODO: calculate from fields
    }

  protected:

    std::vector<Layout *> fields_;

  private:

    static Primitive &
    empty_layout(void)
    {
        static Primitive empty_layout_ {sizeof(std::byte), alignof(std::byte)};
        return empty_layout_;
    }
};
#endif

} // namespace kaleidoscope::type::memory

namespace kaleidoscope
{

namespace traits
{
    template<typename T>
    concept Type_Basic_Scalar = std::is_fundamental_v<T>
                             && std::is_scalar_v<T>
                             && !std::is_pointer_v<T>
                             && !std::is_array_v<T>;

    template<typename T>
    concept Type_Callable =
            std::is_function_v<T> || std::is_invocable_v<T>
            || requires(T t) { &std::decay_t<T>::operator(); };
} // namespace traits
} // namespace kaleidoscope

namespace kaleidoscope::type
{

struct Info
{
    struct Impl
    {
        std::function<void(void *self)>              initialize;
        std::function<void(void *self, void *other)> clone;
        std::function<void(void *self, void *other)> move;
        std::function<void(void *self)>              dispose;
    };

    Info(std::string name, std::unique_ptr<memory::Layout> layout, Impl impl)
        : name_(std::move(name))
        , layout_(std::move(layout))
        , impl_(std::move(impl))
    {
    }

    Info(const Info &)            = delete;
    Info(Info &&)                 = default;
    Info &operator=(const Info &) = delete;
    Info &operator=(Info &&)      = default;

  public:

    bool
    operator==(const Info &other)
    {
        if (this == &other)
        {
            return true;
        }

        return name_ == other.name_; // FIXME: наивно, но если имена типов уникальны - сработает.
    }

  public:

    std::string_view
    name() const
    {
        return {name_};
    }

  protected:

    std::string                     name_;
    std::unique_ptr<memory::Layout> layout_;
    Impl                            impl_;
};

template<traits::Type_Basic_Scalar T>
Info
make_info(std::string name)
{
    auto layout = std::make_unique<memory::Primitive>(sizeof(T), alignof(T));

    const Info::Impl impl {
            .initialize = [size = layout->size()](void *self) -> void
            {
                std::memset(self, 0, size);
            },
            .clone = [size = layout->size()](void *self, void *other) -> void
            {
                std::memcpy(self, other, size);
            },
            .move = [size = layout->size()](void *self, void *other) -> void
            {
                std::memcpy(self, other, size);
            },
            .dispose = [](void *self) -> void
            {
                return;
            },
    };

    return {
            std::move(name),
            std::move(layout),
            std::move(impl)
    };
}

} // namespace kaleidoscope::type
