#include <iostream>
#include <memory>
#include <numeric>

using namespace std;

template<typename T>
class LList {
private:
    struct Node {
        T data;
        unique_ptr<Node> next;
        Node(const T& value) : data(value), next(nullptr) {}
    };

    unique_ptr<Node> head; // служебный узел
    Node* tail;            // последний узел
    size_t count;

public:
    // Конструктор по умолчанию
    LList() : head(make_unique<Node>(T())), tail(nullptr), count(0) {}

    // Деструктор
    ~LList() { clear(); }

    // Конструктор копирования
    LList(const LList& other) : LList() {
        Node* current = other.head->next.get();
        while (current) {
            push_back(current->data);
            current = current->next.get();
        }
    }

    // Оператор присваивания копированием (копируем новый объект в существующий)
    LList& operator=(const LList& other) {
        if (this != &other) {
            clear();
            Node* current = other.head->next.get();
            while (current) {
                push_back(current->data);
                current = current->next.get();
            }
        }
        return *this;
    }

    // Конструктор перемещения
    LList(LList&& other) noexcept
        : head(move(other.head)), tail(other.tail), count(other.count) {
        other.head = make_unique<Node>(T());
        other.tail = nullptr;
        other.count = 0;
    }

    // Оператор присваивания перемещением
    LList& operator=(LList&& other) noexcept {
        if (this != &other) {
            clear();
            head = move(other.head);
            tail = other.tail;
            count = other.count;
            other.head = make_unique<Node>(T());
            other.tail = nullptr;
            other.count = 0;
        }
        return *this;
    }

    // Добавление в конец
    void push_back(const T& value) {
        auto new_node = make_unique<Node>(value);
        Node* new_node_ptr = new_node.get();
        if (!tail) {
            head->next = move(new_node);
            tail = new_node_ptr;
        }
        else {
            tail->next = move(new_node);
            tail = new_node_ptr;
        }
        ++count;
    }

    // Добавление в начало
    void push_front(const T& value) {
        auto new_node = make_unique<Node>(value);
        if (!tail) tail = new_node.get();
        new_node->next = move(head->next);
        head->next = move(new_node);
        ++count;
    }

    // Вставка перед элементом с индексом
    void insert(size_t index, const T& value) {
        if (index > count) throw out_of_range("Index out of range");
        if (index == 0) { push_front(value); return; }
        if (index == count) { push_back(value); return; }

        Node* prev = head.get();
        for (size_t i = 0; i < index; ++i)
            prev = prev->next.get();

        auto new_node = make_unique<Node>(value);
        new_node->next = move(prev->next);
        prev->next = move(new_node);
        ++count;
    }

    // Удаление последнего
    void pop_back() {
        if (!tail) return;
        if (count == 1) {
            head->next.reset();
            tail = nullptr;
        }
        else {
            Node* prev = head.get();
            while (prev->next.get() != tail)
                prev = prev->next.get();
            prev->next.reset();
            tail = prev;
        }
        --count;
    }

    // Удаление первого
    void pop_front() {
        if (!head->next) return;
        head->next = move(head->next->next);
        if (!head->next) tail = nullptr;
        --count;
    }

    // Удаление по индексу
    void remove_at(size_t index) {
        if (index >= count) throw out_of_range("Index out of range");
        if (index == 0) { pop_front(); return; }
        Node* prev = head.get();
        for (size_t i = 0; i < index; ++i)
            prev = prev->next.get();
        if (prev->next.get() == tail) tail = prev;
        prev->next = move(prev->next->next);
        --count;
    }

    // Доступ к элементу по индексу
    T& operator[](size_t index) {
        if (index >= count) throw out_of_range("Index out of range");
        Node* current = head->next.get();
        for (size_t i = 0; i < index; ++i)
            current = current->next.get();
        return current->data;
    }

    const T& operator[](size_t index) const {
        if (index >= count) throw out_of_range("Index out of range");
        Node* current = head->next.get();
        for (size_t i = 0; i < index; ++i)
            current = current->next.get();
        return current->data;
    }

    // Размер списка
    size_t size() const { return count; }

    // Проверка на пустоту
    bool empty() const { return count == 0; }

    // Очистка списка
    void clear() {
        head->next.reset();
        tail = nullptr;
        count = 0;
    }

    // Первый элемент
    const T& front() const {
        if (!head->next) throw out_of_range("List is empty");
        return head->next->data;
    }

    // Последний элемент
    const T& back() const {
        if (!tail) throw out_of_range("List is empty");
        return tail->data;
    }
};

// Функция для печати списка
template<typename T>
void print_lst(const LList<T>& l) {
    for (size_t i = 0; i < l.size(); ++i) {
        cout << l[i];
        if (i + 1 != l.size()) cout << " -> ";
    }
    cout << endl;
}

int main() {
    LList<char> lst;
    cout << boolalpha << lst.empty() << endl;

    for (int i = 0; i < 5; ++i)
        lst.push_back('a' + i);
    print_lst(lst);

    for (int i = 0; i < 5; ++i)
        lst.insert(0, 'z' - i);
    print_lst(lst);

    for (size_t i = 0; i < lst.size(); ++i)
        lst[i] = 'a' + i;
    print_lst(lst);

    lst.pop_back();
    lst.pop_front();
    print_lst(lst);

    lst.remove_at(5);
    lst.insert(3, 'o');
    print_lst(lst);

    lst.clear();
    lst.push_back('q');
    lst.push_back('w');
    cout << lst.size() << " " << boolalpha << lst.empty() << endl;

    // Проверка правила пяти
    LList<char> a;
    for (char c = 'a'; c <= 'e'; ++c) a.push_back(c);

    LList<char> b = a; // copy constructor
    b[0] = 'Z';
    print_lst(a);
    print_lst(b);

    LList<char> c;
    c = a; // copy assignment
    c[1] = 'X';
    print_lst(a);
    print_lst(c);

    LList<char> d = move(a); // move constructor
    print_lst(d);
    cout << "size a = " << a.size() << endl;

    LList<char> e;
    e = move(c); // move assignment
    print_lst(e);

    LList<int> l;
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);
    l.insert(0, 100);
    l.insert(l.size(), 200);
    print_lst(l);

    return 0;
}
