#include <memory>
#include <utility>
#include <iostream>
#include <iterator>
#include <type_traits>

class Range {
public:
    using value_type = int;
    using size_type = std::size_t;

    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

    Range(int s, int e) : lower_bound(s), upper_bound(e) {}
    ~Range() = default;

    template <typename T>
    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;

        using value_type = T;
        using difference_type = std::ptrdiff_t; 

        using pointer = T*; 
        using reference = T&; 

        iterator(T val) : val(val) {}
        ~iterator() = default;
        iterator(const iterator& other) = default;
        iterator& operator=(const iterator& other) = default;

        bool operator!=(const iterator<T>& other) {
            return val != other.val;
        }

        bool operator==(const iterator<T>& other) {
            return val == other.val;
        }

        iterator operator+(value_type v) {
            iterator temp = *this;
            temp.val += v;
            return temp;
        }

        iterator& operator+=(value_type v) {
            val += v;
            return *this;
        }

        iterator& operator++() {
            ++val;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++val;
            return temp;
        }

        iterator& operator--() {
            --val;
            return *this;
        }

        iterator operator--(int) {
            iterator temp = *this;
            --val;
            return temp;
        }

        value_type operator*() {
            return val;
        }

    private:
        T val;
    };

    template <typename T>
    using reverse_iterator = std::reverse_iterator<iterator<T>>;

    iterator<value_type> begin() {
        return iterator{lower_bound};
    }

    iterator<value_type> end() {
        return iterator{upper_bound+1};
    }

    reverse_iterator<value_type> rbegin() {
        return reverse_iterator<value_type>{ end() };
    }

    reverse_iterator<value_type> rend() {
        return reverse_iterator<value_type>{ begin() };
    }

private:
    value_type lower_bound;
    value_type upper_bound;
};

int main() {
    Range a{1, 5};

    for (auto it = a.rbegin(); it != a.rend(); ++it) {
        std::cout << "val is: " << *it << std::endl;
    }

    return 0;
}