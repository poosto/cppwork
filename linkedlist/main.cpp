#include <memory>
#include <iostream>
#include <ostream>

template <typename T, class Allocator = std::allocator<T>>
class LinkedList {
public:
    // Remember: these types are outward-facing
    // We don't want the user to interact with the Node type, only the underlying stored type

    using value_type = T;

    struct Node {
        value_type data;
        Node* next;
        
        Node(value_type data, Node* next) : data(data), next(next) {}
    };

    using allocator_type = std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using size_type = std::size_t;

    using pointer = T*;
    using const_pointer = const T*;

    using reference = T&;
    using const_reference = const T&;

    // Default constructor
    explicit LinkedList(const allocator_type& alloc = allocator_type{}) : allocator(alloc), head(nullptr), tail(nullptr) {}

    // Fill constructor
    explicit LinkedList(size_type n, value_type default_value = value_type{}, const allocator_type& alloc = allocator_type{}) :
        LinkedList(alloc) 
    {
        for (size_type i = 0; i < n; ++i) {
            push_back(default_value);
        }
    }

    // Destructor
    ~LinkedList() {
        while (head) {
            pop_front();
        }
    }

    // Copy constructor
    LinkedList(const LinkedList& other) :
        LinkedList(other.allocator)
    {
        Node* node = other.head;

        while (node) {
            push_back(node->data);
            node = node->next;
        }
    }

    // Copy assignment operator
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            LinkedList temp(other);
            swap(temp);
        }

        return *this;
    }

    // Move constructor
    LinkedList(LinkedList&& other) :
        allocator(std::move(other.allocator)),
        head(std::exchange(other.head, nullptr)),
        tail(std::exchange(other.tail, nullptr)) {}

    // Move assignment operator
    LinkedList& operator=(LinkedList&& other) {
        if (this != &other) {
            LinkedList temp(std::move(other));
            swap(temp);
        }

        return *this;
    }

    // Modifier methods
    void push_back(value_type val) {
        // Allocate a new node and construct it
        Node* node = std::allocator_traits<allocator_type>::allocate(allocator, 1);
        std::allocator_traits<allocator_type>::construct(allocator, node, Node{val, nullptr});

        // Assign head if needed
        if (!head) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = tail->next;
        }
    }

    void push_front(value_type val) {
        // Allocate a new node and construct it
        Node* node = std::allocator_traits<allocator_type>::allocate(allocator, 1);
        std::allocator_traits<allocator_type>::construct(allocator, node, Node{val, nullptr});

        // Assign head if needed
        if (!head) {
            head = node;
            tail = node;
        } else {
            node->next = head;
            head = node;
        }
    }

    void pop_front() {
        if (head) {
            Node* next = head->next;

            std::allocator_traits<allocator_type>::destroy(allocator, head);
            std::allocator_traits<allocator_type>::deallocate(allocator, head, 1);
            
            head = next;
        }
    }

    class iterator {
    public:
        iterator(Node* node) : node(node) {}
        ~iterator() = default;

        iterator& operator++() {
            if (node) {
                node = node->next;
            }

            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;

            if (node) {
                node = node->next;
            }

            return temp;
        }

        bool operator==(const iterator& other) const {
            return this->node == other.node;
        }

        bool operator!=(const iterator& other) const {
            return this->node != other.node;
        }

        reference operator*() {
            return node->data;
        } 

        const_reference operator*() const {
            return node->data;
        }

    private:
        Node* node;
    };

    iterator begin() {
        return iterator{ head };
    }

    iterator end() {
        return iterator{ nullptr };
    }

    friend std::ostream& operator<<(std::ostream& o, LinkedList& obj) {
        o << "hello: " << obj.head << std::endl;
        return o;
    }

private:
    allocator_type allocator;
    Node* head;
    Node* tail;

    void swap(LinkedList& other) {
        std::swap(allocator, other.allocator);
        std::swap(head, other.head);
        std::swap(tail, other.tail);
    }
};

int main() {
    LinkedList<int> a{5, 15};
    LinkedList<int> b(a);

    for (auto it = a.begin(); it != a.end(); ++it) {
        *it = 6;
        std::cout << *it << std::endl;
    }

    for (int x : b) {
        std::cout << x << std::endl;
    }

    LinkedList<int> c = b;
    LinkedList<int> d;
    d = std::move(c);

    std::cout << d << std::endl;

    return 0;
}