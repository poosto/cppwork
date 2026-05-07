#include <cstdint>
#include <iostream>
#include <memory>

template <typename T>
class Optional {
using Allocator = std::allocator<T>;
using AllocTrait = std::allocator_traits<Allocator>;

public:
    // Empty constructor: no value
    Optional() : has_value_(false) {}

    // Value constructor
    Optional(const T& value) : has_value_(true) {
        Allocator alloc;
        AllocTrait::construct(alloc, reinterpret_cast<T*>(buf_), value);
        // new (buf_) T{value};
    }

    // Copy constructor
    Optional(const Optional& other) : has_value_(other.has_value_) {
        if (has_value_) {
            Allocator alloc;
            AllocTrait::construct(alloc, reinterpret_cast<T*>(buf_), *reinterpret_cast<const T*>(other.buf_));
        }
    }

    // Move constructor
    Optional(Optional&& other) : has_value_(std::exchange(other.has_value_, false)) {
        if (has_value_) {
            Allocator alloc;
            AllocTrait::construct(alloc, reinterpret_cast<T*>(buf_), std::move(*reinterpret_cast<T*>(other.buf_)));
            AllocTrait::destroy(alloc, reinterpret_cast<T*>(other.buf_));
        }
    }

    // Destructor
    ~Optional() {
        if (has_value_) {
            Allocator alloc;
            AllocTrait::destroy(alloc, reinterpret_cast<T*>(buf_));
            // reinterpret_cast<T*>(buf_)->~T();
        }
    }

    // Access value
    const T& value_or(const T& other) {
        if (has_value_) {
            return *reinterpret_cast<T*>(buf_);
        } else {
            return other;
        }
    }

private:
    bool has_value_{};
    alignas(T) uint8_t buf_[sizeof(T)];
};

using std::cout, std::endl;

int main() {
    Optional<std::string> a;
    Optional<std::string> b("Hello world");
    Optional<std::string> c = b;
    Optional<std::string> d = std::move(c);
    
    cout << a.value_or("default") << " " << b.value_or("default") << endl;
    cout << c.value_or("default") << " " << d.value_or("default") << endl;

    return 0;
}