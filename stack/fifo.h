#include <cstdlib>
#include <initializer_list>
#include <memory>

template <typename T, size_t Capacity> class FIFO {
  union Slot;

public:
  using value_type = T;
  using pointer_type = T *;
  using reference_type = T &;
  using const_reference_type = const T &;
  using size_type = size_t;

  // TODO: make non-const iterator as well
  // perhaps template an iterator struct on <bool Const> ?
  struct const_iterator {
    using value_type = T;
    using difference_type = ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    constexpr const_iterator() = default;
    constexpr const_iterator(const Slot *ptr, size_t idx, size_t max_idx)
        : ptr_{ptr}, idx_{idx}, max_idx_{max_idx} {}
    constexpr ~const_iterator() = default;

    constexpr const_iterator &operator++() {
      if (idx_ < max_idx_) {
        ++idx_;
      }

      return *this;
    }

    constexpr const_iterator operator++(int) {
      auto tmp = *this;

      if (idx_ < max_idx_) {
        ++idx_;
      }

      return tmp;
    }

    constexpr bool operator==(const const_iterator &other) const {
      return idx_ == other.idx_;
    }

    constexpr bool operator!=(const const_iterator &other) const {
      return idx_ != other.idx_;
    }

    constexpr const_reference_type operator*() const {
      return ptr_[idx_].value;
    }

    const Slot *ptr_{};
    size_t idx_{};
    size_t max_idx_{};
  };

  constexpr FIFO() : size_{0} {}

  constexpr FIFO(std::initializer_list<T> list) : size_{list.size()} {
    size_t i = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
      std::construct_at(std::addressof(data_[i++].value), *it);
    }
  }

  constexpr FIFO(const FIFO &other) : size_{other.size()} {
    for (size_t i = 0; i < size_; ++i) {
      std::construct_at(std::addressof(data_[i].value), other.data_[i].value);
    }
  }

  constexpr FIFO &operator=(const FIFO &other) {
    if (this != &other) {
      FIFO temp{other};
      swap(temp);
    }

    return *this;
  }

  constexpr ~FIFO() {
    for (size_t i = 0; i < size_; ++i) {
      std::destroy_at(std::addressof(data_[i].value));
    }
  }

  constexpr void push(const_reference_type val) {
    // TODO: throw/assert/something?
    if (size_ >= Capacity)
      return;

    std::construct_at(std::addressof(data_[size_++].value), val);
  }

  constexpr void pop() {
    // TODO throw/assert/something?
    if (size_ == 0)
      return;

    std::destroy_at(std::addressof(data_[--size_].value));
  }

  constexpr value_type top() { return data_[size_ - 1].value; }

  constexpr size_t size() const { return size_; }
  consteval size_t capacity() const { return Capacity; }
  constexpr const_reference_type peek() const { return data_[size_ - 1].value; }

  constexpr const_iterator begin() const {
    return const_iterator{data_, 0, size_};
  }

  constexpr const_iterator end() const {
    return const_iterator{data_, size_, size_};
  }

private:
  // Can't use std::array since this holds actual T objects
  // std::array<std::byte, sizeof(T) * Capacity> data_;

  // Remember that trivial types are left indeterminate if not explicitly
  // default constructed. This is unlike non-trivial types with defined
  // constructors. So adding {} here zeros the buffer for no reason!
  // alignas(T) std::byte data_[sizeof(T) * Capacity];

  // Crazy union trick that avoids default initialization of non-trivial members
  // Unions avoid default construction of internal values
  // Also, you can't do one anonymous union with T[Capacity] inside,
  // because constexpr constructors track the creation of unions,
  // and the entire array must be default constructed for it to be considered
  // 'active'. So if you try to set data_[i], you're accessing a non-active
  // union! errors like: construction of subobject of member 'test_' of union
  // with no active member is not allowed in a constant expression.
  // By having an array of unions, each entry of the array has its own state
  // and you can activate the entire union by
  // "placement-new/std::construct_at"-ing each element.

  union Slot {
    constexpr Slot() {}
    constexpr ~Slot() {}

    T value;
  };

  Slot data_[Capacity];
  size_t size_{};

  constexpr void swap(FIFO &other) {
    data_swap(other);

    std::swap(size_, other.size_);
  }

  constexpr void data_swap(FIFO &other) {
    size_t min_size = std::min(size_, other.size_);

    // Swap elements from both FIFOs (all constructed)
    for (size_t i = 0; i < min_size; ++i) {
      std::swap(data_[i].value, other.data_[i].value);
    }

    // Move any existing elements (if other.size_ > size_)
    for (size_t i = min_size; i < other.size_; ++i) {
      std::construct_at(std::addressof(data_[i].value),
                        std::move(other.data_[i].value));
      std::destroy_at(std::addressof(other.data_[i].value));
    }

    // Move any existing elements (if size_ > other.size_)
    for (size_t i = min_size; i < size_; ++i) {
      std::construct_at(std::addressof(other.data_[i].value),
                        std::move(data_[i].value));
      std::destroy_at(std::addressof(data_[i].value));
    }
  }
};
