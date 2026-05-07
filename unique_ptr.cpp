template <typename T>
class UniquePtr {
private:
    T* ptr;

public:
    // Constructor
    // Doing new T(data) instead of new T; ensures that T does NOT need a default constructor
    UniquePtr(T data) : ptr(new T(data)) {}

    // Destructor
    ~UniquePtr() {
        delete ptr;
    }

    // Copy constructor + Copy assignment operator
    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& other) = delete;

    // Move constructor + Move assignment operator
    UniquePtr(UniquePtr&& other) {
        // this->ptr = other.ptr
        // other.ptr = nullptr
        this->ptr = std::exchange(other.ptr, nullptr);
    }

    UniquePtr& operator=(UniquePtr&& other) {
        // Avoid unnecessary moves and memory leak
        if (this != &other) {
            delete this->ptr;
            this->ptr = std::exchange(other.ptr, nullptr);
        }

        return *this;
    }
};