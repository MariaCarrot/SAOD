#include <iostream>
#include <stdexcept>

using namespace std;

template<typename T>
class LList {
private:
    struct Node {
        T data;
        Node* next;

        Node(const T& value) : data(value), next(nullptr) {}
    };

    Node* head;      // служебный узел
    size_t count;    // количество настоящих элементов

public:
    // Конструктор по умолчанию
    LList() : head(new Node(T())), count(0) {
        head->next = head;
    }

    // Деструктор
    ~LList() {
        clear();
        delete head;
    }

    // Конструктор копирования
    LList(const LList& other) : LList() {
        Node* current = other.head->next;

        while (current != other.head) {
            push_back(current->data);
            current = current->next;
        }
    }

    // Оператор присваивания копированием
    LList& operator=(const LList& other) {
        if (this != &other) {
            clear();

            Node* current = other.head->next;

            while (current != other.head) {
                push_back(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Конструктор перемещения
    LList(LList&& other) noexcept
        : head(other.head), count(other.count) {

        other.head = new Node(T());
        other.head->next = other.head;
        other.count = 0;
    }

    // Оператор присваивания перемещением
    LList& operator=(LList&& other) noexcept {
        if (this != &other) {
            clear();
            delete head;

            head = other.head;
            count = other.count;

            other.head = new Node(T());
            other.head->next = other.head;
            other.count = 0;
        }
        return *this;
    }

    // Добавление в конец списка
    void push_back(const T& value) { insert(count, value); }

    // Добавление в начало списка
    void push_front(const T& value) { insert(0, value); }

    // Вставка перед элементом с индексом index
    void insert(size_t index, const T& value) {
        if (index > count) throw out_of_range("Index out of range");

        Node* prev = head;
        for (size_t i = 0; i < index; ++i) 
            prev = prev->next;

        Node* new_node = new Node(value);
        new_node->next = prev->next;
        prev->next = new_node;
        ++count;
    }

    // Удаление последнего элемента
    void pop_back() {
        if (empty()) return;
        remove_at(count - 1);
    }

    // Удаление первого элемента
    void pop_front() {
        if (empty()) return;
        remove_at(0);
    }

    // Удаление элемента по индексу
    void remove_at(size_t index) {
        if (index >= count) throw out_of_range("Index out of range");

        Node* prev = head;

        for (size_t i = 0; i < index; ++i) 
            prev = prev->next;

        Node* node_to_delete = prev->next;
        prev->next = node_to_delete->next;
        delete node_to_delete;
        --count;
    }

    // Доступ к элементу по индексу
    T& operator[](size_t index) {
        if (index >= count) throw out_of_range("Index out of range");

        Node* current = head->next;
        for (size_t i = 0; i < index; ++i) 
            current = current->next;
        return current->data;
    }

    // Константный доступ к элементу по индексу
    const T& operator[](size_t index) const {
        if (index >= count) throw out_of_range("Index out of range");

        Node* current = head->next;
        for (size_t i = 0; i < index; ++i)
            current = current->next;
        return current->data;
    }

    // Размер списка
    size_t size() const { return count; }

    // Проверка на пустоту
    bool empty() const { return count == 0; }

    // Очистка списка
    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    // Первый элемент
    const T& front() const {
        if (empty()) throw out_of_range("List is empty");
        return head->next->data;
    }

    // Последний элемент
    const T& back() const {
        if (empty()) throw out_of_range("List is empty");

        Node* current = head->next;
        while (current->next != head) 
            current = current->next;
        return current->data;
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
        lst.push_back(char('a' + i));

    print_lst(lst);

    for (int i = 0; i < 5; ++i)
        lst.insert(0, char('z' - i));

    print_lst(lst);

    for (size_t i = 0; i != lst.size(); ++i)
        lst[i] = char('a' + i);

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

    cout << lst.size() << ' ' << boolalpha << lst.empty() << endl;

    // Проверка правила пяти

    LList<char> a;

    for (char c = 'a'; c <= 'e'; ++c)
        a.push_back(c);

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
