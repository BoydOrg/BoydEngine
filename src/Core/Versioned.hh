#pragma once

#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

namespace boyd
{

/// A shared pointer that keeps track of all modifications to its wrapped value,
/// storing a version number of them.
template <typename T>
class Versioned
{
    struct Data
    {
        T value;
        std::atomic<unsigned> version;

        Data(T &&value)
            : value{std::move(value)}, version{1}
        {
        }
        ~Data() = default;
    };
    std::shared_ptr<Data> data;

public:
    /// Constructs a new, null `Versioned<T>`.
    Versioned(std::nullptr_t null = nullptr)
        : data{nullptr}
    {
    }

    /// Constructs a new `T` from the given `TArgs`, wrapping it in a `Versioned<T>` with version=1.
    template <typename... TArgs>
    static Versioned<T> Make(TArgs &&... args)
    {
        Versioned<T> self;
        self.data = std::make_shared<Data>(T{std::forward<TArgs>(args)...});
        return self;
    }

    Versioned(const Versioned &toCopy) = default;
    Versioned &operator=(const Versioned &toCopy) = default;
    Versioned(Versioned &&toMove) = default;
    Versioned &operator=(Versioned &&toMove) = default;

    ~Versioned() = default;

    inline operator bool() const
    {
        return bool(data);
    }
    inline bool operator==(const Versioned &other) const
    {
        return data == other.data;
    }
    inline bool operator!=(const Versioned &other) const
    {
        return data != other.data;
    }

    /// read-only access to the wrapped value.
    /// does not change the version number.
    inline const T &operator*() const
    {
        return data->value;
    }

    /// Read-only access to the wrapped value. Returns null if the value is not present.
    /// Does not change the version number.
    inline const T *operator->() const
    {
        if(data)
        {
            return &data->value;
        }
        return nullptr;
    }

    /// Read-only access to the wrapped value. Returns null if the value is not present.
    /// Does not change the version number.
    inline const T *Get() const
    {
        if(data)
        {
            return &data->value;
        }
        return nullptr;
    }

    /// Read-write access to the wrapped value: calls the given function, passing it a pointer to the wrapped value
    /// (or null if no value is present), then increments the version number.
    /// Returns `this` for method chaining.
    template <typename TFunction, typename = std::enable_if<std::is_invocable<TFunction, T *>::value>>
    inline Versioned *Edit(TFunction &function)
    {
        if(data)
        {
            function(&data->value);
            data->version++;
        }
        else
        {
            function(nullptr);
        }
        return this;
    }

    /// Returns the current version number of the wrapped data.
    /// Returns 0 if the Versioned<T> is null.
    inline unsigned version() const
    {
        if(data)
        {
            return data->version;
        }
        return 0;
    }
};

} // namespace boyd