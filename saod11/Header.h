#pragma once
#ifndef LIST_H
#define LIST_H
struct Node {
    char data;
    struct Node* next;
};

struct Metrics {
    long comparisons;
    long pointer_changes;
};

void create_list(struct Node** p);
void push_back(struct Node** p, char value);
int size(const struct Node* p);
void print_list(const struct Node* p);
void remove_list(struct Node** p);

void push_front(struct Node** p, char value);
void insert_node(struct Node** p, int index, char value);

void pop_back(struct Node** p);
void pop_front(struct Node** p);
void remove_node(struct Node** p, int index);

void clear(struct Node** p);
#endif