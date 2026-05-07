template <typename T>
class Vector {
private:
    constexpr size_t DEFAULT_SIZE = 4;
    constexpr size_t SCALE_FACTOR = 2;

    T* data; // I think T[] is also okay
    size_t len;

public:
    Vector() : len(0) {
        data = new T[DEFAULT_SIZE];
    }

    ~Vector() {
        delete[] data;
    }

    // Copy constructor
    Vector(const Vector& other) : data(new T[len]), len(other.len) {
        for (size_t i = 0; i < len; ++i) {
            data[i] = other.data[i];
        }
    }

    // Copy assignment operator
    Vector& operator=(const Vector& other) {
        if (this == &other) {
            return *this;
        }

        // By creating a temporary object,
        // we reuse the code in the copy constructor AND the destructor.
        // No need to manually delete[] again or copy data.
        //
        // EDIT: better to pass other by value, 
        // because it automatically invokes copy constructor.

        Vector temp(other);
        std::swap(this->data, temp.data);
        std::swap(this->len, temp.len);

        return *this;
    }

    // Move constructor
    // Member initializer lists avoid unnecessary default constructor calls
    Vector(Vector&& other) : data(other.data), len(other.len) {
        other.data = nullptr;
        other.len = 0;
    }

    // Move assignment operator
    Vector& operator=(Vector&& other) {
        if (this == &other) {
            return *this;
        }

        Vector temp(std::move(other));
        std::swap(this->data, temp.data);
        std::swap(this->len, temp.len);

        return *this;
    }
};