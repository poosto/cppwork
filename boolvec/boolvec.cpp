#include <memory>
#include <utility>
#include <iostream>
#include <iterator>
#include <type_traits>

template <typename T, class Allocator = std::allocator<T> >
class Vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using allocator_type = Allocator;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using iterator = pointer;
    using const_iterator = const_pointer;

    // Constructors
    Vector() : Vector(allocator_type{}) {}
    explicit Vector(const allocator_type& alloc) : allocator(alloc), capacity(0), idx(0), data(nullptr) {}

    explicit Vector(size_type n, const_reference default_value = value_type{}, const allocator_type& alloc = allocator_type{}) :
        allocator(alloc), capacity(n), idx(n),
        data(std::allocator_traits<allocator_type>::allocate(allocator, n))
    {
        for (size_type i = 0; i < capacity; ++i) {
            std::allocator_traits<allocator_type>::construct(allocator, &data[i], default_value);
        }
    }

    template <class InputIt,
              typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
    Vector(InputIt first, InputIt last, const allocator_type& alloc = allocator_type{}) : allocator(alloc)
    {
        // typename std::iterator_traits<InputIt>::difference_type n = std::distance(first, last);
        size_type n = std::distance(first, last);

        capacity = n;
        idx = n;
        data = std::allocator_traits<allocator_type>::allocate(allocator, n);

        size_type i = 0;
        for (auto it = first; it != last; ++i, ++it) {
            std::allocator_traits<allocator_type>::construct(allocator, &data[i], *it);
        }
    }

    // Destructor
    ~Vector() {
        for (size_type i = 0; i < capacity; ++i) {
            std::allocator_traits<allocator_type>::destroy(allocator, &data[i]);
        }

        std::allocator_traits<allocator_type>::deallocate(allocator, data, capacity);
    }

    // Copy constructor
    Vector(const Vector& other) :
        allocator(other.allocator),
        capacity(other.capacity),
        idx(other.idx),
        data(std::allocator_traits<allocator_type>::allocate(allocator, other.capacity))
    {
        for (size_type i = 0; i < capacity; ++i) {
            std::allocator_traits<allocator_type>::construct(allocator, &data[i], other.data[i]);
        }
    }

    // Copy assignment operator
    Vector& operator=(const Vector& other) {
        // this can never equal other, since other is a copy
        // regardless, copying yourself is technically safe, just a waste of computation
        if (this != &other) {
            Vector temp{other};
            swap(temp);
        }

        return *this;
    }

    // TODO: propagate_on_container_copy_assignment and for move constructor as well

    // Move constructor
    Vector(Vector&& other) :
        allocator(std::move(other.allocator)),
        capacity(std::exchange(other.capacity, 0)),
        idx(std::exchange(other.idx, 0)),
        data(std::exchange(other.data, nullptr)) {}

    // Move assignment operator
    Vector& operator=(Vector&& other) {
        if (this != &other) {
            Vector temp(std::move(other));
            swap(temp);
        }

        return *this;
    }

    // Iterator methods
    iterator begin() { return data; }
    iterator end() { return data + idx; }
    const_iterator begin() const { return data; }
    const_iterator end() const { return data + idx; }

    reference operator[](size_type index) {
        return data[index];
    }

    const_reference operator[](size_type index) const {
        return data[index];
    }

    size_type size() const {
        return idx;
    }

    iterator emplace(const_iterator pos, value_type val) {
        size_type i = std::distance(begin(), pos);

        if (i + 1 >= capacity) {
            // in other impls, we would grow, but for now this is fine.
            return end();
        }

        // If the local index is beyond the constructed memory, simply construct in-place
        // Otherwise, we need to move all elements after it
        if (i < idx) {
            for (size_type j = i; j < idx; ++j) {
                std::allocator_traits<allocator_type>::construct(
                    allocator,
                    &data[j + 1]
                    data[j]
                );

                std::allocator_traits<allocator_type>::destroy(
                    allocator,
                    &data[j]
                );
            }
        }

        std::allocator_traits<allocator_type>::construct(
            allocator,
            &data[idx],
            val
        );
    }

private:
    allocator_type allocator;
    size_type capacity;
    size_type idx;
    pointer data;

    void swap(Vector& other) {
        std::swap(data, other.data);
        std::swap(capacity, other.capacity);
        std::swap(idx, other.idx);
        std::swap(allocator, other.allocator);
    }
};

// TODO
template <class Allocator>
class Vector<bool, Allocator> {
public:
    class proxy;

    using value_type = bool;
    using size_type = std::size_t;

    // rebind std::allocator<bool> to std::allocator<size_t>
    using allocator_type = typename std::allocator_traits<Allocator>
        ::template rebind_alloc<size_type>;

    using pointer = size_type*;
    using const_pointer = const size_type*;
    using reference = proxy;
    using const_reference = const proxy;

    // Constructors
    Vector() : Vector(allocator_type{}) {}
    explicit Vector(const allocator_type& alloc) : allocator(alloc), capacity(0), idx(0), bit(0), data(nullptr) {}

    explicit Vector(size_type n, value_type default_value = value_type{}, const allocator_type& alloc = allocator_type{}) :
        allocator(alloc)
    {
        // ceil(n / size_t)
        size_type data_size = (n + sizeof(size_t) - 1) / sizeof(size_t);

        capacity = data_size;
        idx = n / sizeof(size_t);
        bit = n % sizeof(size_t);
        data = std::allocator_traits<allocator_type>::allocate(allocator, data_size);

        for (size_type i = 0; i < capacity; ++i) {
            if (default_value) {
                std::allocator_traits<allocator_type>::construct(allocator, &data[i], 0xFF);
            } else {
                std::allocator_traits<allocator_type>::construct(allocator, &data[i], 0);
            }
        }
    }

    // Destructor
    ~Vector() {
        for (size_type i = 0; i < capacity; ++i) {
            std::allocator_traits<allocator_type>::destroy(allocator, &data[i]);
        }

        std::allocator_traits<allocator_type>::deallocate(allocator, data, capacity);
    }

    reference operator[](size_type index) {
        return reference{ data[index / sizeof(size_type)], index % sizeof(size_type) };
    }

    const_reference operator[](size_type index) const {
        return reference{ data[index / sizeof(size_type)], index % sizeof(size_type) };
    }

    size_type size() const {
        return (idx * sizeof(size_type)) + bit;
    }

private:
    allocator_type allocator;
    size_type capacity;
    size_type idx;
    size_type bit;
    pointer data;
};

template <class Allocator>
class Vector<bool, Allocator>::proxy {
public:
    proxy(size_t& word, size_t bit) : word(word), bit(bit) {}

    operator bool() const {
        return (word >> bit) & 1;
    }

    proxy& operator=(const proxy& other) {
        return *this = bool(other);
    }

    proxy& operator=(bool b) {
        if (b) {
            word |= (1 << bit);
        } else {
            word &= ~(1 << bit);
        }

        return *this;
    }

private:
    size_t& word;
    size_t bit;
};

int main() {
    Vector<int> a;
    Vector<int> b(4);
    Vector<int> c(4, 10);
    Vector<int> d(b);
    Vector<int> e = d;
    Vector<int> f(std::move(e));
    Vector<int> g = std::move(f);

    /*for (auto it = c.begin(); it != c.end(); ++it) {
        std::cout << *it << std::endl;
    }

    c[1] = 5;

    const Vector<int> h(c.begin(), c.begin() + 2);

    for (int element : h) {
        std::cout << element << std::endl;
    }*/

    Vector<bool> test(129, false);
    std::cout << test.size() << std::endl;
    std::cout << test[5] << std::endl;

    test[7] = true;
    std::cout << test[7] << test[8] << std::endl;

    return 0;
}