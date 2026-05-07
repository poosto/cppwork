template <typename T>
class Stack {
private:
    T* data;
    size_t size;
    size_t capacity;

    static constexpr size_t INITIAL_SIZE = 4;
    static constexpr size_t SCALE_FACTOR = 2;

    void resize() {
        capacity *= SCALE_FACTOR;

        T* temp = new T[capacity];
        for (size_t i = 0; i < size; ++i) {
            temp[i] = data[i];
        }

        delete[] data;
        data = temp;
    }

    void swap(Stack& other) {
        std::swap(data, other.data);
        std::swap(size, other.size);
        std::swap(capacity, other.capacity);
    }

public:
    void push(T element) {
        if (size == capacity) {
            resize();
        }

        data[size++] = element;
    }

    void pop() {
        if (size == 0) {
            return;
        }

        --size;
    }

    // User must check if stack is empty beforehand
    T& top() {
        return data[size - 1];
    }

    Stack() : data(new T[INITIAL_SIZE]), size(0), capacity(INITIAL_SIZE) {}
    
    ~Stack() {
        delete[] data;
    }

    Stack(const Stack& other) : data(new T[other.capacity]), size(other.size), capacity(other.capacity) {
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }

    Stack& operator=(Stack other) {
        swap(other);
        return *this;
    }

    Stack(Stack&& other) : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }


    // While passing by copy invokes the copy constructor,
    // passing by rvalue ref does NOT invoke the move constructor,
    // since no new object is created. It's just a ref!
    Stack& operator=(Stack&& other) {
        if (this != &other) {
            Stack temp(std::move(other));
            swap(temp);
        }
        
        return *this;
    }
};