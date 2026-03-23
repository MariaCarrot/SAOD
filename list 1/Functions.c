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
        printf("%c", cur->data);
        if (cur->next != p)
            printf(" -> ");
        cur = cur->next;
    }

    printf("\n");
}

void push_back(struct Node** p, char value)
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

void push_front(struct Node** p, char value)
{
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));

    if (new_node == NULL) exit(1);

    new_node->data = value;
    new_node->next = (*p)->next;
    (*p)->next = new_node;
}

void insert_node(struct Node** p, int index, char value)
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