template <typename T>
class SharedPtr {
private:
    struct Metadata {
        size_t ref_count;
    };

    T* ptr;
    Metadata* metadata;

    void swap(SharedPtr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(metadata, other.metadata);
    }

public:
    SharedPtr(T data) : ptr(new T(data)), metadata(new Metadata) {
        metadata->ref_count = 1;
    }

    // Destructor
    ~SharedPtr() {
        if (metadata) {
            --metadata->ref_count;

            if (metadata->ref_count == 0) {
                delete ptr;
                delete metadata;
            }
        }
    }

    // Copy constructor
    SharedPtr(const SharedPtr& other) : ptr(other.ptr), metadata(other.metadata) {
        ++metadata->ref_count;
    }

    // Copy assignment operator
    // Pass by value to automatically invoke the copy constructor
    // No need for temp object
    // It will then invoke the destructor for the original data inside this
    // Tradeoff: you still create a copy even if this == &other
    // Benefit: let the C++ engine handle copying the object, which is generally better
    SharedPtr& operator=(SharedPtr other) {
        // Cannot check if this == &other because this is a local copy

        // This is equivalent to SharedPtr temp(other); then swapping w/ the temp variable
        swap(other);

        return *this;
    }

    // Move constructor
    SharedPtr(SharedPtr&& other) : ptr(std::exchange(other.ptr, nullptr)), metadata(std::exchange(other.metadata, nullptr)) {}

    // Move assignment operator
    // Remember: for assignment operators, must be aware of the old, existing data
    SharedPtr& operator=(SharedPtr&& other) {
        if (this != &other) {
            // reuse move constructor + destructor
            SharedPtr temp(std::move(other));
            swap(temp);
        }

        return *this;
    } 
};