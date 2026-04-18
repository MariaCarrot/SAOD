#include <stdio.h>
#include <stdlib.h>
#include "Header.h"

void create_list(struct Node** p)
{
    *p = (struct Node*)malloc(sizeof(struct Node));
    if (*p == NULL) exit(1);

    (*p)->next = *p;
}

int size(const struct Node* p)
{
    int count = 0;
    const struct Node* cur = p->next;

    while (cur != p)
    {
        count++;
        cur = cur->next;
    }

    return count;
}

void print_list(const struct Node* p)
{
    const struct Node* cur = p->next;

    while (cur != p)
    {
        printf("%d", cur->data);
        if (cur->next != p)
            printf(" -> ");
        cur = cur->next;
    }

    printf("\n");
}

void push_back(struct Node** p, int value)
{
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
    struct Node* cur = *p;

    if (new_node == NULL) exit(1);

    new_node->data = value;

    while (cur->next != *p)
        cur = cur->next;

    cur->next = new_node;
    new_node->next = *p;
}

void push_front(struct Node** p, int value)
{
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));

    if (new_node == NULL) exit(1);

    new_node->data = value;
    new_node->next = (*p)->next;
    (*p)->next = new_node;
}

void insert_node(struct Node** p, int index, int value)
{
    struct Node* cur = *p;
    struct Node* new_node;
    int i;

    if (index < 0) return;

    for (i = 0; i < index && cur->next != *p; i++)
        cur = cur->next;

    new_node = (struct Node*)malloc(sizeof(struct Node));
    if (new_node == NULL) exit(1);

    new_node->data = value;
    new_node->next = cur->next;
    cur->next = new_node;
}

void pop_front(struct Node** p)
{
    struct Node* tmp;

    if ((*p)->next == *p) return;

    tmp = (*p)->next;
    (*p)->next = tmp->next;
    free(tmp);
}

void pop_back(struct Node** p)
{
    struct Node* cur = *p;
    struct Node* prev = NULL;

    if ((*p)->next == *p) return;

    while (cur->next != *p)
    {
        prev = cur;
        cur = cur->next;
    }

    prev->next = *p;
    free(cur);
}

void remove_node(struct Node** p, int index)
{
    struct Node* cur = *p;
    struct Node* tmp;
    int i;

    if (index < 0) return;

    for (i = 0; i < index && cur->next != *p; i++)
        cur = cur->next;

    if (cur->next == *p) return;

    tmp = cur->next;
    cur->next = tmp->next;
    free(tmp);
}

void clear(struct Node** p)
{
    struct Node* cur = (*p)->next;
    struct Node* tmp;

    while (cur != *p)
    {
        tmp = cur;
        cur = cur->next;
        free(tmp);
    }

    (*p)->next = *p;
}

void remove_list(struct Node** p)
{
    clear(p);
    free(*p);
    *p = NULL;
}


// Генерация списков
void generate_random(struct Node** p, int n, int min, int max)
{
    for (int i = 0; i < n; i++)
    {
        int value = min + rand() % (max - min + 1);
        push_back(p, value);
    }
}

void generate_sorted(struct Node** p, int n)
{
    for (int i = 0; i < n; i++)
    {
        push_back(p, i);
    }
}

void generate_reverse_sorted(struct Node** p, int n)
{
    for (int i = n - 1; i >= 0; i--)
    {
        push_back(p, i);
    }
}

struct Node* get_node(struct Node* p, int index)
{
    struct Node* cur = p->next;

    for (int i = 0; i < index && cur != p; i++)
        cur = cur->next;

    return (cur == p) ? NULL : cur;
}

void generate_almost_sorted(struct Node** p, int n)
{
    // сначала делаем отсортированный
    generate_sorted(p, n);

    int swaps = n * (5 + rand() % 6) / 100; // 5–10%

    for (int i = 0; i < swaps; i++)
    {
        int idx1 = rand() % n;
        int idx2 = rand() % n;

        struct Node* node1 = get_node(*p, idx1);
        struct Node* node2 = get_node(*p, idx2);

        if (node1 && node2)
        {
            int tmp = node1->data;
            node1->data = node2->data;
            node2->data = tmp;
        }
    }
}

// сортировка вставкой 
void insertion_sort(struct Node* head, struct Metrics* m)
{
    struct Node* i;
    struct Node* j;
    struct Node* prev_j;
    struct Node* next_i;

    if (head->next == head || head->next->next == head)
        return;

    i = head->next->next;

    while (i != head)
    {
        next_i = i->next;

        j = head->next;
        prev_j = head;

        while (j != i)
        {
            m->comparisons++;

            if (j->data > i->data)
                break;

            prev_j = j;
            j = j->next;
        }

        if (j != i)
        {
            struct Node* prev_i = head;

            while (prev_i->next != i)
                prev_i = prev_i->next;

            prev_i->next = i->next;
            m->pointer_changes++;

            i->next = j;
            prev_j->next = i;
            m->pointer_changes += 2;
        }

        i = next_i;
    }
}


// 1. Слияние цепочек узлов с подсчетом
struct Node* merge_nodes(struct Node* a, struct Node* b, struct Metrics* m) {
    struct Node dummy;
    struct Node* tail = &dummy;

    while (a != NULL && b != NULL) {
        m->comparisons++; // Сравниваем данные

        if (a->data <= b->data) {
            tail->next = a;
            a = a->next;
        }
        else {
            tail->next = b;
            b = b->next;
        }
        m->pointer_changes++; // Изменили tail->next
        tail = tail->next;
    }

    // Прицепляем оставшийся хвост
    if (a != NULL || b != NULL) {
        tail->next = (a != NULL) ? a : b;
        m->pointer_changes++;
    }

    return dummy.next;
}

// 2. Разделение списка
void split_list(struct Node* source, struct Node** front, struct Node** back, struct Metrics* m) {
    struct Node* fast;
    struct Node* slow;
    slow = source;
    fast = source->next;

    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *front = source;
    *back = slow->next;
    slow->next = NULL; // Разрыв списка — это изменение указателя
    m->pointer_changes++;
}

// 3. Рекурсивная сортировка
void merge_sort_recursive(struct Node** head_ref, struct Metrics* m) {
    struct Node* head = *head_ref;
    struct Node* a;
    struct Node* b;

    if ((head == NULL) || (head->next == NULL)) {
        return;
    }

    split_list(head, &a, &b, m);

    merge_sort_recursive(&a, m);
    merge_sort_recursive(&b, m);

    *head_ref = merge_nodes(a, b, m);
    m->pointer_changes++; // Обновление head_ref
}

// 4. Основная функция
struct Metrics merge_sort(struct Node** p) {
    struct Metrics m = { 0, 0 };

    if ((*p)->next == *p) return m;

    // Линеаризация (превращаем кольцо в цепь)
    struct Node* head = (*p)->next;
    struct Node* cur = head;
    while (cur->next != *p) {
        cur = cur->next;
    }
    cur->next = NULL;
    m.pointer_changes++;

    // Сортировка
    merge_sort_recursive(&head, &m);

    // Восстановление кольца
    (*p)->next = head;
    m.pointer_changes++;

    cur = head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = *p;
    m.pointer_changes++;

    return m;
}
