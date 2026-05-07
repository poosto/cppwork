#include <memory>
#include <utility>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <list>
#include <utility>

namespace my {

template <typename T, class Allocator = std::allocator<T>>
class Vector {
public:
    // Outward-facing types
    using value_type = T;
    using size_type = size_t;
    using allocator_type = Allocator;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using iterator = pointer;
    using const_iterator = const iterator;

    // Default constructor
    explicit Vector(const allocator_type& alloc = allocator_type{}) : 
        alloc_(alloc) {}

    // Fill constructor
    explicit Vector(size_type n, value_type default_value = value_type{}, const allocator_type& alloc = allocator_type{}) :
        capacity_(n),
        size_(n),
        alloc_(alloc)
    {
        data_ = std::allocator_traits<allocator_type>::allocate(alloc_, capacity_);
        fill_with(n, default_value);
    }

    // Initializer list constructor
    // Purposely left as implicit so you can do Vector<T> = { .. }
    Vector(std::initializer_list<T> list, const allocator_type& alloc = allocator_type{}) :
        capacity_(list.size()),
        size_(list.size()),
        alloc_(alloc)
    {
        data_ = std::allocator_traits<allocator_type>::allocate(alloc_, capacity_);
        fill_with(list.begin(), list.end()); 
    }

    // Iterator constructor
    template <class InputIt>
    Vector(InputIt begin, InputIt end, const allocator_type& alloc = allocator_type{}) :
        capacity_(std::distance(begin, end)),
        size_(std::distance(begin, end)),
        alloc_(alloc)
    {
        data_ = std::allocator_traits<allocator_type>::allocate(alloc_, capacity_);
        fill_with(begin, end);
    }

    // Destructor
    ~Vector() {
        for (size_type i = 0; i < capacity_; ++i) {
            std::allocator_traits<allocator_type>::destroy(
                alloc_,
                &data_[i]
            );
        }

        std::allocator_traits<allocator_type>::deallocate(alloc_, data_, capacity_);
    }

    // Copy constructor
    Vector(const Vector& other) :
        size_(other.size_),
        capacity_(other.capacity_),
        alloc_(other.alloc_)
    {
        data_ = std::allocator_traits<allocator_type>::allocate(alloc_, capacity_);
        fill_with(other.begin(), other.end()); 
    }

    // Copy assignment operator
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector temp(other);
            swap(other);
        }

        return *this;
    }

    // Move constructor
    Vector(Vector&& other) :
        size_(std::exchange(other.size_, 0)),
        capacity_(std::exchange(other.capacity_, 0)),
        alloc_(std::move(other.alloc_)),
        data_(std::exchange(other.data_, nullptr))
    {}

    // Move assignment operator
    Vector& operator=(Vector&& other) {
        if (this != &other) {
            Vector temp(std::move(other));
            swap(temp);
        }

        return *this;
    }

    // Iterators //

    iterator begin() {
        return &data_[0];
    }

    iterator end() {
        return &data_[size_];
    }

    // Operator overloads //

    reference operator[](size_type i) {
        return data_[i];
    }

    // Modifiers //

    void push_back(value_type&& value) {
        try_grow();
        
        std::allocator_traits<allocator_type>::construct(
            alloc_,
            &data_[size_++],
            std::forward<value_type>(value)
        );
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        try_grow();

        std::allocator_traits<allocator_type>::construct(
            alloc_,
            &data_[size_++],
            std::forward<Args&&>(args)...
        );
    }

private:
    size_type capacity_{};
    size_type size_{};

    allocator_type alloc_{};

    pointer data_{};

    void swap(Vector& other) {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(alloc_, other.alloc_);
        std::swap(data_, other.data_);
    }

    template <class InputIt>
    void fill_with(InputIt begin, InputIt end) {
        for (auto it = begin; it != end; ++it) {
            auto idx = std::distance(begin, it);
            
            std::allocator_traits<allocator_type>::construct(
                alloc_,
                &data_[idx],
                *it
            );
        }
    }

    void fill_with(size_type n, value_type value) {
        for (size_type i = 0; i < n; ++i) {
            std::allocator_traits<allocator_type>::construct(
                alloc_,
                &data_[i],
                value
            );
        }
    }

    void resize(size_type new_capacity) {
        pointer temp = std::allocator_traits<allocator_type>::allocate(
            alloc_,
            new_capacity
        );

        for (size_type i = 0; i < size_; ++i) {
            std::allocator_traits<allocator_type>::construct(
                alloc_,
                &temp[i],
                data_[i]
            );
        }

        for (size_type i = 0; i < size_; ++i) {
            std::allocator_traits<allocator_type>::destroy(
                alloc_,
                &data_[i]
            );
        }

        std::allocator_traits<allocator_type>::deallocate(alloc_, data_, capacity_);

        data_ = temp;
        capacity_ = new_capacity;
    }

    void try_grow() {
        static constexpr size_type GROWTH_FACTOR = 2;

        if (capacity_ == 0) {
            resize(1);
        }

        if (size_ >= capacity_) {
            resize(capacity_ * GROWTH_FACTOR);
        }
    }
};

}

template <typename T>
void test(T foo) {
    foo += 1;
}

template <typename T>
class ref_wrap {
public:
    ref_wrap(T& obj) : data_(obj) {}

    operator T&() {
        return data_;
    }

private:
    T& data_;
};

struct Hello {
    int x[4];
};

template <typename T>
void bar(T&& baz) {
    baz = 45;
}

// void bar(Hello baz) {

// }

int main() {
    using std::cout, std::endl;

    Hello x{};
    const Hello& y = x;
    bar(y);
    // bar(std::move(x));

    /*int x = 4;
    ref_wrap x_wrap(x);
    test(x_wrap);
    cout << x << endl;

    my::Vector<char> a(4, 'c');
    my::Vector<char> b{'a', 'b', 'c', 'd'};

    std::list<char> tmp = {'a', 'x', 'y', 'z'};
    my::Vector<char> c{tmp.begin(), tmp.end()};

    for (int i = 0; i < 4; ++i) {
        cout << a[i] << b[i] << c[i] << endl;
    }

    my::Vector<int> d{1, 2, 3, 4, 5};
    for (int element : d) {
        cout << element;
    }
    cout << endl;*/

    my::Vector<int> testA;
    testA.push_back(1);
    testA.push_back(2);
    testA.push_back(3);
    for (int element : testA) {
        cout << element;
    }
    cout << endl;

    return 0;
}
