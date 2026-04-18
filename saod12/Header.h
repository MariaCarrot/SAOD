#pragma once
#ifndef LIST_H
struct Node {
    int data;
    struct Node* next;
};

struct Metrics {
    long comparisons;
    long pointer_changes;
};

void create_list(struct Node** p);
void push_back(struct Node** p, int value);
int size(const struct Node* p);
void print_list(const struct Node* p);
void remove_list(struct Node** p);

void push_front(struct Node** p, int value);
void insert_node(struct Node** p, int index, int value);

void pop_back(struct Node** p);
void pop_front(struct Node** p);
void remove_node(struct Node** p, int index);

void clear(struct Node** p);

void generate_random(struct Node** p, int n, int min, int max);
void generate_sorted(struct Node** p, int n);
void generate_reverse_sorted(struct Node** p, int n);
struct Node* get_node(struct Node* p, int index);
void generate_almost_sorted(struct Node** p, int n);

void insertion_sort(struct Node* head, struct Metrics* m);

struct Node* merge_nodes(struct Node* a, struct Node* b, struct Metrics* m);
void split_list(struct Node* source, struct Node** front, struct Node** back, struct Metrics* m);
void merge_sort_recursive(struct Node** head_ref, struct Metrics* m);
struct Metrics merge_sort(struct Node** p);
#endif
